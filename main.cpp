// ImGui - standalone example application for Glfw + OpenGL 3, using programmable pipeline
// If you are new to ImGui, see examples/README.txt and documentation at the top of imgui.cpp.

#include <imgui.h>
#include "imgui_impl_glfw_gl3.h"
#include <stdio.h>
#include <GL/gl3w.h>    // This example is using gl3w to access OpenGL functions (because it is small). You may use glew/glad/glLoadGen/etc. whatever already works for you.
#include <GLFW/glfw3.h>
#include <cmath>

#include <chrono>
#include <memory>
#include <thread>
#include <vector>

static void error_callback(int error, const char* description)
{
  fprintf(stderr, "Error %d: %s\n", error, description);
}



struct PerformanceBlockData
{
  const char *mName;
  const char *mFile;
  int mLine;
};

struct PerformanceData
{
  PerformanceBlockData *mBlock;

  // How many levels below this are there.
  //size_t      mDepth;

  // What level are we at?
  //size_t      mLevel;

  std::chrono::time_point<std::chrono::high_resolution_clock> mBeginTime;
  std::chrono::time_point<std::chrono::high_resolution_clock> mEndTime;

  std::vector<PerformanceData> mChildren;

  PerformanceData(PerformanceBlockData *aBlock)
    : mBlock(aBlock)
      //mDepth(0),
      //mLevel(0)
  {
    mBeginTime = std::chrono::high_resolution_clock::now();
  }

  PerformanceData(PerformanceData &&aOther)
    : mBlock(aOther.mBlock),
      //mDepth(aOther.mDepth),
      //mLevel(aOther.mLevel),
      mBeginTime(aOther.mBeginTime),
      mEndTime(aOther.mEndTime),
      mChildren(std::move(aOther.mChildren))
  {

  }

  void End()
  {
    mEndTime = std::chrono::high_resolution_clock::now();
  }

  ~PerformanceData()
  {
    End();
  }

  double TimeTaken()
  {
    std::chrono::duration<double, std::milli> fp_ms = mEndTime - mBeginTime;

    return fp_ms.count();
  }
};

struct PerformanceRecorder : public PerformanceData
{
  static PerformanceRecorder gRoot;
  static PerformanceRecorder *gLastPerformanceCheck;

  PerformanceRecorder(PerformanceBlockData *aBlock)
    : PerformanceData(aBlock),
      mParent(gLastPerformanceCheck)
  {
    gLastPerformanceCheck = this;

    //mLevel = mParent->mLevel + 1;
  }

  ~PerformanceRecorder()
  {
    if (this == mParent)
    {
      return;
    }

    gLastPerformanceCheck = mParent;
    PerformanceData *self = this;

    End();

    mParent->mChildren.emplace_back(std::move(*self));

    //if (mParent->mDepth < (mDepth + 1))
    //{
    //  mParent->mDepth = mDepth + 1;
    //}
  }

  PerformanceRecorder *mParent;
};

PerformanceBlockData gBlockPerf{ "Global Root", __FILE__, __LINE__ };

PerformanceRecorder *PerformanceRecorder::gLastPerformanceCheck = &gRoot;
PerformanceRecorder PerformanceRecorder::gRoot{ &gBlockPerf };


#define YTRACE(aName, x, y) \
  static PerformanceBlockData __block__##x##y{aName, __FILE__, __LINE__};\
  PerformanceRecorder __perf__##x##y {&__block__##x##y};
  //PerformanceBlockData *__block__##x##y = GetPerformanceBlock<decltype(aName), aName, decltype(__FILE__), __FILE__, __LINE__>(); 
#define XTRACE(aName, x, y) YTRACE(aName, x, y)
#define TRACE(aName, x) XTRACE(aName, x, __COUNTER__)

#define RecordPerf(aName) TRACE(aName, __RECORDER__)


void DisplayPerf(PerformanceData *aRoot, double aTimeScale, size_t aCount = 0)
{
  for (auto &perf : aRoot->mChildren)
  {
    ImGui::PushID(&perf);
    ImGui::SameLine();


    ImGui::PushItemWidth(0.0f);
    ImGui::BeginGroup();

    float hue = aCount*0.05f;

    auto size = ImVec2(static_cast<float>(aTimeScale * perf.TimeTaken()), 0.0f);

    ImGui::PushStyleColor(ImGuiCol_Header, ImColor::HSV(hue, 0.6f, 0.6f));
    ImGui::Selectable(perf.mBlock->mName, true, 0, size);

    ImGui::PopStyleColor(1);

    if (ImGui::IsItemHovered())
    {
      ImGui::SetTooltip("%s\n"
                        "Location: %s:%d\n"
                        "Children: %d\n"
                        "Total Time: %fms\n"
                        "Size: %f\n",
                        perf.mBlock->mName,
                        perf.mBlock->mFile,
                        perf.mBlock->mLine,
                        perf.mChildren.size(),
                        perf.TimeTaken(),
                        size.x);
    }
    ImGui::NewLine();
    DisplayPerf(&perf, aTimeScale, aCount + 1);
    ImGui::EndGroup();
    ImGui::PopItemWidth();
    ImGui::PopID();
  }
}

void ShowPerfWindowTest1()
{
  ImGui::Begin("Performance Analyser Test 1");

  static double timeWidthScaling = 1.0f;
  const double zoomSpeed = 1.15f;

  float wheel = ImGui::GetIO().MouseWheel;


  ImGui::BeginChild("scrolling", ImVec2(0.0f, 0.0f), true, ImGuiWindowFlags_HorizontalScrollbar);



  if (0.0f > wheel)
  {
    timeWidthScaling /= zoomSpeed;
  }
  else if (0.0f < wheel)
  {
    timeWidthScaling *= zoomSpeed;
  }

  DisplayPerf(&PerformanceRecorder::gRoot, timeWidthScaling);

  //static float lastScrollMax = ImGui::GetScrollMaxX();
  //float currentScrollMax = ImGui::GetScrollMaxX();
  //
  //static float lastScroll = ImGui::GetScrollX();
  //float currentScroll = ImGui::GetScrollX();

  //float ratio = lastScroll / lastScrollMax;

  //float scroll = ratio * currentScrollMax;

  //printf("L: %f LM: %f C: %f CM: %f S:%f\n",
  //       lastScroll,
  //       lastScrollMax,
  //       currentScroll,
  //       currentScrollMax,
  //       scroll);

  if (ImGui::IsWindowFocused() && ImGui::IsMouseDragging())
  {
    ImVec2 offset(0.0f, 0.0f);

    offset.x -= ImGui::GetIO().MouseDelta.x;
    offset.y -= ImGui::GetIO().MouseDelta.y;

    ImGui::SetScrollX(ImGui::GetScrollX() + offset.x);
    ImGui::SetScrollY(ImGui::GetScrollY() + offset.y);
  }
  //else if (lastScrollMax != currentScrollMax)
  //{
  //  //ImGui::SetScrollX(scroll);
  //
  //  lastScrollMax = currentScrollMax;
  //  lastScroll = currentScroll;
  //}

  ImGui::EndChild();
  ImGui::End();
}



void DrawPerfItem(PerformanceData &aData)
{
  auto initialWindowPos = ImGui::GetCursorScreenPos();
  //initialWindowPos.x += 1.0f; 
  //initialWindowPos.y += 1.0f;

  ImVec4 clip_rect(0.0f, 0.0f, 0.0f, 0.0f);
  
  //ImVec2 textSize(aData.mTotalTime, ImGui::GetFontSize());
  ImVec2 boxSize(aData.TimeTaken() / 2, ImGui::GetTextLineHeight());

  ImGui::Dummy(initialWindowPos, boxSize);
  if (ImGui::IsItemHovered())
  {
    ImGui::SetTooltip("%s\n"
                      "Location: %s:%d\n"
                      "Children: %d\n"
                      "Total Time: %fms\n",
                      aData.mBlock->mName,
                      aData.mBlock->mFile,
                      aData.mBlock->mLine,
                      aData.mChildren.size(),
                      aData.TimeTaken());
  }

  printf("x: %f, y: %f\n", initialWindowPos.x, initialWindowPos.y);

  ImGui::GetWindowDrawList()->AddRectFilled(initialWindowPos, 
                                            boxSize,
                                            ImColor(90, 90, 120, 255));

  ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), 
                                      ImGui::GetFontSize(), 
                                      initialWindowPos, 
                                      ImColor(255, 255, 255, 255), 
                                      aData.mBlock->mName);
}



void ShowPerfWindowTest2()
{
  ImGui::Begin("Performance Analyser Test 2");

  static double timeWidthScaling = 1.0f;
  const double zoomSpeed = 1.15f;

  float wheel = ImGui::GetIO().MouseWheel;

  if (0.0f > wheel)
  {
    timeWidthScaling /= zoomSpeed;
  }
  else if (0.0f < wheel)
  {
    timeWidthScaling *= zoomSpeed;
  }

  DrawPerfItem(PerformanceRecorder::gRoot.mChildren[0]);

  if (ImGui::IsWindowFocused() && ImGui::IsMouseDragging())
  {
    ImVec2 offset(0.0f, 0.0f);

    offset.x -= ImGui::GetIO().MouseDelta.x;
    offset.y -= ImGui::GetIO().MouseDelta.y;

    ImGui::SetScrollX(ImGui::GetScrollX() + offset.x);
    ImGui::SetScrollY(ImGui::GetScrollY() + offset.y);
  }

  ImGui::End();
}



using namespace std::chrono_literals;

void GenerateRecursivePerf(size_t aTimes)
{
  RecordPerf("GenerateRecursive");

  std::this_thread::sleep_for(2ms);

  if (0 == aTimes)
  {
    return;
  }

  GenerateRecursivePerf(aTimes - 1);
  GenerateRecursivePerf(aTimes / 2);
  GenerateRecursivePerf(aTimes / 3);
  GenerateRecursivePerf(aTimes / 4);
}


double GenerateNSquaredPerf(size_t aTimes)
{
  RecordPerf("GenerateNSquared");

  double squaredSquared = aTimes;

  for (size_t i = 1; i <= aTimes; ++i)
  {
    RecordPerf("OuterLoop");
    for (size_t j = 1; j <= aTimes; ++j)
    {
      RecordPerf("InnerLoop");
      squaredSquared *= aTimes;
      std::this_thread::sleep_for(2ms);
    }
  }

  return squaredSquared;
}


void GenerateTimeData()
{
  RecordPerf("GenerateTimeData");

  double d = GenerateNSquaredPerf(7);
  (void)d;
  GenerateRecursivePerf(12);
}


int main(int, char**)
{
  GenerateTimeData();

  // Setup window
  glfwSetErrorCallback(error_callback);
  if (!glfwInit())
    return 1;
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  #if __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  #endif
  GLFWwindow* window = glfwCreateWindow(1280, 720, "ImGui OpenGL3 example", NULL, NULL);
  glfwMakeContextCurrent(window);
  gl3wInit();

  // Setup ImGui binding
  ImGui_ImplGlfwGL3_Init(window, true);

  // Load Fonts
  // (there is a default font, this is only if you want to change it. see extra_fonts/README.txt for more details)
  //ImGuiIO& io = ImGui::GetIO();
  //io.Fonts->AddFontDefault();
  //io.Fonts->AddFontFromFileTTF("../../extra_fonts/Cousine-Regular.ttf", 15.0f);
  //io.Fonts->AddFontFromFileTTF("../../extra_fonts/DroidSans.ttf", 16.0f);
  //io.Fonts->AddFontFromFileTTF("../../extra_fonts/ProggyClean.ttf", 13.0f);
  //io.Fonts->AddFontFromFileTTF("../../extra_fonts/ProggyTiny.ttf", 10.0f);
  //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());


  ImVec4 clear_color = ImColor(114, 144, 154);

  // Main loop
  while (!glfwWindowShouldClose(window))
  {
    glfwPollEvents();
    ImGui_ImplGlfwGL3_NewFrame();

    ImGui::SetNextWindowPos(ImVec2(350, 20), ImGuiSetCond_FirstUseEver);
    ShowPerfWindowTest1();
    ShowPerfWindowTest2();


    // Rendering
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui::Render();
    glfwSwapBuffers(window);
  }

  // Cleanup
  ImGui_ImplGlfwGL3_Shutdown();
  glfwTerminate();

  return 0;
}

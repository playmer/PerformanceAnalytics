# PerformanceAnalytics
Working on a Performance Analyser using ImGui. It's pretty early, but it kindof works. I'd like to eventually make a few different ways to interact with it. One where it runs at it's own application and can take and look at various formats of data, perhaps where it can take live data from a socket. Maybe one where you can just use it with your own ImGui application and it's more of a library. I dunno!

#Issues
- Currently there's no "time" display. How the blocks are displayed is mostly based on however dear ImGui figures out where to put the next object on a line/next line based on how big they are and how they're being grouped. I'm not yet sure how to make this an "absolute" thing like what @ocornut (The creator of dear ImGui) does in his game engine or how the Telemetry folks do it.

  - A sub problem of that (something that would seemingly be easier to fix in the short-term) is that currently each grouping seems to indent slightly.
  - Another sub problem is that Selectables (what's being used to display each block) seem to have a miniumum size, and just odd size properties in general. So the sizes are skewed the smaller I go.

- If a block is partially offscreen (to the left) the beginning of the text doesn't get moved to the start of the screen, it just stays happily offscreen.
- Scrolling isn't normalized when zooming in and out. I suspect I'll fix this quickly (Next time I work on it), I'm just screwing up the math.
- And so many more.

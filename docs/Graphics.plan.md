- implement two different methods of terminal/graphics handling. Implement a command line option to chose.
- no-graphics mode:
  - implement all things that can be done with xterm escapes or equivalent on other systems
  - every attempt to invoke a graphics mode should cause a runtime error
- graphics mode:
  - implement all the various graphics and text modes described
  - we open a window in 280x192 pixel format; add a command line uption for scaling on screen
  - text (40+24 or 80x24 with additional horizontl scaling to fit in the same window) uses 
    "The Ultimate Apple II Font" (fetch from https://www.kreativekorp.com/software/fonts/apple2/,
     also download the charset map linked from the same page).
     For text output in this mode, we need to implement everything ourselves
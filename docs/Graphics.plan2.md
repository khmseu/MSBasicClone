## Plan: Graphics Mode Selection and Terminal Abstraction

Implement command-line mode selection to separate no-graphics (terminal-only) and graphics (windowed) implementations. Current graphics is off-screen only; the goal is to add actual rendering and optional terminal-only fallback.

### Steps

1. **Add command-line mode selection** — Extend [main.cpp](src/main.cpp) with `--no-graphics` and `--graphics` flags; default to no-graphics. Pass mode config to `Interpreter`.

2. **Conditionally register graphics statements** — Wrap graphics command tokens (GR, HGR, PLOT, HPLOT, etc.) in tokenizer/parser behind mode check or factory pattern; raise runtime error in no-graphics mode if attempted.

3. **Separate terminal I/O from graphics** — In no-graphics mode, all output uses xterm escape sequences (already partially done via ANSI codes). In graphics mode, implement SDL2 or similar windowed display with 280×192 pixel buffer.

4. **Implement Ultimate Apple II Font rendering** — For graphics mode, download font from kreativekorp.com and implement text glyph rendering into the pixel buffer; support 40×24 and 80×24 text modes with horizontal scaling to fit window.

5. **Add scaling option** — Support `--scale N` command-line flag for graphics window size multiplier; apply to both text and graphics coordinates.

6. **Write no-graphics mode test** — Add test that attempts graphics command and verifies runtime error is thrown; ensure all PRINT/terminal features work without graphics.

### Further Considerations

1. **Graphics rendering backend** — Choose SDL2 (cross-platform, minimal deps) or Raylib? SDL2 is mature; Raylib is lighter. Both have font/texture support. / Need library decision before coding pixel buffer rendering.

- Raylib.

1. **No-graphics error behavior** — Should graphics commands fail hard (throw error, stop program) or silently ignore? Applesoft would execute; stub mode should probably error for strict compatibility testing.

- Runtime error like all other runtime errors.

1. **Text mode scaling in graphics mode** — 40×24 on 280×192 requires per-character pixel mapping. Should each char be 7×8 or 8×8 pixels? The Ultimate Apple II Font is typically 7×8; confirm this matches the 280-pixel width.

- Yes, 7x8.

1. **PEEK/POKE in graphics mode** — Current `peekMemory` / `pokeMemory` read/write a simulated memory map. In graphics mode, should reads to video memory (e.g., `PEEK(16384)`) return actual pixel buffer data, or remain stub?

- At the moment, video memory access with peek/poke is not necessary. However, graphic mode switch peek/poke should be supported (likely via a switch statement)
- Also, pr#3 switches to 80x24 text mode and pr#0 switches back to 40x24 / 280x192 or mixed modes.

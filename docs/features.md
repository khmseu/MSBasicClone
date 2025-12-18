# Features

## Implemented

- Core flow: tokenizer → parser → interpreter sharing the same AST for interactive and program execution.
- Numeric runtime: custom 40-bit floating point (`src/float40.*`) with number/string `Value` type; expression support for +, -, \*, /, ^, MOD, unary ±/NOT, comparisons, AND/OR; integer variables (`%` suffix) clamp to 16-bit signed range.
- Statements: PRINT/? with separators; INPUT with optional prompt; LET/assign; IF/THEN[/ELSE] inline or to line numbers; GOTO; GOSUB/RETURN; FOR/NEXT with STEP; WHILE/WEND for conditional loops; CLR to reset variables/loops; END; REM comment; COLON separators; TRACE/NOTRACE to echo executed line numbers; RANDOMIZE seed to initialize RND; SPEED n (clamped 0–255 ms) to insert per-statement delay; PR#n/IN#n stubs store device slots while keeping screen/keyboard active; POP to remove entry from GOSUB stack; WAIT addr,mask to poll I/O; HIMEM=/LOMEM= to set memory boundaries; HPLOT for high-resolution graphics plotting (points and lines); MOVE x,y for cursor positioning in hi-res graphics; ROTATE angle and SCALE n for shape parameters; DRAW shape [AT x,y] and XDRAW shape [AT x,y] stubs that accept syntax and record coordinates (shape table not yet implemented).
- Error trapping: ONERR GOTO line; RESUME to continue after handler.
- Data handling: DATA collected before run; READ consumes sequentially across control flow; RESTORE resets the DATA cursor (optionally to a target line); DATA honored in immediate mode.
- Arrays: DIM with expression subscripts; auto-dimension to size 10 per dimension if undeclared; shared access/assign path; bounds checks yield BAD SUBSCRIPT.
- User-defined functions: DEF FNname(arg)=expr parsed to AST; functions stored in `Variables`; one-argument calls `FNx(expr)` evaluated with local parameter substitution.
- Built-in functions: SIN, COS, TAN, ATN, EXP, LOG, SQR, ABS, INT, SGN, RND, LEN, VAL, ASC, CHR$, LEFT$, RIGHT$, MID$, STR$, PEEK(), SCRN(x,y) (stub returns 0), USR(addr) (stub returns 0).
- CLI commands: RUN (optionally `RUN n`), LIST (optionally `LIST start[,end]`), NEW, LOAD filename, SAVE filename, CATALOG, CLR; immediate-mode statements use the same parser/interpreter.
- Cursor helpers: TAB(n) and SPC(n) return space padding; POS reports current column on terminals (ANSI query) and Windows consoles; falls back to 0 otherwise.
- Output notes: CHR$(7) emits terminal bell (`\a`) and flushes immediately.
- Graphics stubs: GR/HIRES configure a scaled overlay window sized to the terminal and track scaled plot samples (with color). Basic shape table support: default shapes (triangle, square) available; `DRAW`/`XDRAW` apply rotation (`ROTATE`) and scale (`SCALE`) and record transformed vertices. Still no on-screen rendering.
- Low-level system calls: PEEK/POKE implemented with simple in-memory model; GET to read a keystroke; CALL is a no-op; USR(addr) stub returns 0 (machine language call).

## Yet to Implement / Missing

- Advanced graphics rendering: Actual on-screen rendering and shape table drawing for DRAW/XDRAW (currently stubs that record coordinates only).
- Shape table management: Loading, storing, and interpreting shape tables; honoring current ROTATE/SCALE settings during DRAW/XDRAW.
- Screen/graphics rendering: No on-screen rendering for plotting primitives; SCRN(x,y) returns 0.
- Memory/state queries: FRE returns fixed free-memory placeholder; PDL returns 0; HIMEM/LOMEM track boundaries but don't enforce allocation.
- Additional file or device I/O beyond LOAD/SAVE/CATALOG not implemented.
- WAIT addr,mask polls indefinitely; real Applesoft would support timeout and external I/O events.

- Advanced graphics: DRAW and XDRAW for shape table drawing with rotation/scale (stubs parse but don't render); Shape table loading and management not implemented.
- Screen/graphics rendering: No on-screen rendering for graphics commands or plotting primitives yet (coordinates are recorded but not displayed); SCRN(x,y) stub returns 0.
- Memory/state queries: FRE returns fixed free-memory placeholder; PDL returns 0; HIMEM/LOMEM track boundaries but don't enforce allocation.
- Additional file or device I/O beyond LOAD/SAVE/CATALOG not implemented.
- WAIT addr,mask polls indefinitely; real Applesoft would support timeout and external I/O events.

## Notes

- Variable names are significant to the first two characters (case-insensitive); `$` suffix denotes strings; `%` suffix clamps to 16-bit signed integer.
- DATA is pre-collected before program execution; READ on exhausted data raises OUT OF DATA.
- Keep new tests as `.bas` programs under `tests/` and run with `ctest --test-dir build`.

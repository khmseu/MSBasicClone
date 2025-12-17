# Features

## Implemented

- Core flow: tokenizer → parser → interpreter sharing the same AST for interactive and program execution.
- Numeric runtime: custom 40-bit floating point (`src/float40.*`) with number/string `Value` type; expression support for +, -, \*, /, ^, MOD, unary ±, comparisons, AND/OR.
- Statements: PRINT/? with separators; INPUT with optional prompt; LET/assign; IF/THEN[/ELSE] inline or to line numbers; GOTO; GOSUB/RETURN; FOR/NEXT with STEP; CLR to reset variables/loops; END; REM comment; COLON separators.
- Error trapping: ONERR GOTO line; RESUME to continue after handler.
- Data handling: DATA collected before run; READ consumes sequentially across control flow; RESTORE resets the DATA cursor; DATA honored in immediate mode.
- Arrays: DIM with expression subscripts; auto-dimension to size 10 per dimension if undeclared; shared access/assign path; bounds checks yield BAD SUBSCRIPT.
- User-defined functions: DEF FNname(arg)=expr parsed to AST; functions stored in `Variables`; one-argument calls `FNx(expr)` evaluated with local parameter substitution.
- Built-in functions: SIN, COS, TAN, ATN, EXP, LOG, SQR, ABS, INT, SGN, RND, LEN, VAL, ASC, CHR$, LEFT$, RIGHT$, MID$, STR$.
- CLI commands: RUN, LIST, NEW, LOAD filename, SAVE filename, CATALOG, CLR; immediate-mode statements use the same parser/interpreter.
- Cursor helpers: TAB(n) and SPC(n) return space padding; POS reports current column on terminals (ANSI query) and Windows consoles; falls back to 0 otherwise.
- Graphics stubs: GR/HIRES configure a scaled overlay window sized to the terminal and track scaled plot samples for future drawing APIs (no on-screen rendering yet).

## Yet to Implement / Missing

- Low-level system calls: PEEK/POKE/GET/CALL implemented (in-memory PEEK/POKE; CALL no-op).
- Screen/graphics: No on-screen rendering for graphics commands or plotting primitives yet.
- Memory/state queries: FRE returns fixed free-memory placeholder; PDL returns 0.
- Additional file or device I/O beyond LOAD/SAVE/CATALOG not implemented.

## Notes

- Variable names are significant to the first two characters (case-insensitive); `$` suffix denotes strings; `%` suffix reserved but not separately handled yet.
- DATA is pre-collected before program execution; READ on exhausted data raises OUT OF DATA.
- Keep new tests as `.bas` programs under `tests/` and run with `ctest --test-dir build`.

# Features

## Implemented

- Core flow: tokenizer → parser → interpreter sharing the same AST for interactive and program execution.
- Numeric runtime: custom 40-bit floating point (`src/float40.*`) with number/string `Value` type; expression support for +, -, \*, /, ^, MOD, unary ±, comparisons, AND/OR.
- Statements: PRINT/? with separators; INPUT with optional prompt; LET/assign; IF/THEN[/ELSE] inline or to line numbers; GOTO; GOSUB/RETURN; FOR/NEXT with STEP; END; REM comment; COLON separators.
- Data handling: DATA collected before run; READ consumes sequentially across control flow; RESTORE resets the DATA cursor; DATA honored in immediate mode.
- Arrays: DIM with expression subscripts; auto-dimension to size 10 per dimension if undeclared; shared access/assign path; bounds checks yield BAD SUBSCRIPT.
- User-defined functions: DEF FNname(arg)=expr parsed to AST; functions stored in `Variables`; one-argument calls `FNx(expr)` evaluated with local parameter substitution.
- Built-in functions: SIN, COS, TAN, ATN, EXP, LOG, SQR, ABS, INT, SGN, RND, LEN, VAL, ASC, CHR$, LEFT$, RIGHT$, MID$, STR$.
- CLI commands: RUN, LIST, NEW, LOAD \<file\>, SAVE \<file\>, CATALOG; immediate-mode statements use the same parser/interpreter.

## Yet to Implement / Missing

- Error trapping: ONERR/RESUME tokens exist but no parser/interpreter handling.
- Low-level system calls: PEEK/POKE/GET/CALL tokens exist but no execution support.
- Screen/graphics: HOME, TEXT, GR, HIRES tokens not interpreted.
- Cursor/spacing functions: TAB, SPC, POS tokens not interpreted.
- Memory/state queries: FRE, PDL tokens not interpreted.
- Additional file or device I/O beyond LOAD/SAVE/CATALOG not implemented.

## Notes

- Variable names are significant to the first two characters (case-insensitive); `$` suffix denotes strings; `%` suffix reserved but not separately handled yet.
- DATA is pre-collected before program execution; READ on exhausted data raises OUT OF DATA.
- Keep new tests as `.bas` programs under `tests/` and run with `ctest --test-dir build`.

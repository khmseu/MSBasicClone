# Applesoft BASIC Feature Outline

This is a concise feature map of Applesoft BASIC as implemented (or planned) in this repository. It focuses on syntax shapes, the meaning of commands/functions, and notes needed to emulate behavior.

## Program structure & execution

- Lines: Optional decimal line numbers; stored in ascending order. Blank line number deletes that line. Multiple statements separated by `:` on the same line.
- Immediate mode: A line without a line number executes directly (uses same parser/interpreter). `RUN` starts at lowest line (or specific line if given by `GOTO`/`GOSUB`).
- Flow control state: `GOTO`, `GOSUB`/`RETURN`, `FOR`/`NEXT`, `ONERR`/`RESUME` all mutate interpreter state (program counter, stacks, error handler).

## Variables & data model

- Names: First two characters are significant; case-insensitive; optional suffix `$` (string) or `%` (integer-style token, though values are stored as float internally). User functions (`FNx`) preserve the first three characters.
- Types: Numeric (40-bit float implementation) and string. Uninitialized numeric defaults to `0`; string to empty.
- Arrays: Declared with `DIM A(10)` etc. Indices are numeric expressions. If used before `DIM`, auto-dimensioned to size 10 in each referenced dimension. Subscripting is zero-based; BASIC expression determines index.
- Functions: `DEF FNname(var)=expression` stores a single-argument user function. Arguments are evaluated at call; local argument temporarily shadows global variable.

## Data statements

- `DATA v1, v2, ...` literals (numbers or quoted strings). Collected before execution. In immediate mode, `DATA` values append to the runtime pool.
- `READ v1, v2, ...` consumes sequentially; errors with OUT OF DATA if exhausted.
- `RESTORE` resets the DATA pointer to the start.

## Input/output

- `PRINT` or `?` prints expressions. Separators: `;` concatenates, `,` outputs a field/tab stop (implementation prints a single space), end-of-statement adds newline unless last separator suppresses it.
- `INPUT ["prompt";] V1[,V2...]` prints optional prompt then `?` for each variable. Strings are accepted as-is; numbers are parsed with `stod`, `?REENTER` on failure.
- `GET V` reads a single character from stdin without waiting for newline. Stores character code in numeric vars; a one-char string in string vars.
- `TAB(n)`, `SPC(n)` return a string of `n` spaces. `POS(0)` queries cursor column (ANSI/WinAPI when available, otherwise 0).
- Graphics text control: `HOME` clears screen; `TEXT` sets text mode; `GR` enters low-res graphics; `HIRES` enters high-res graphics. (Currently stubbed/no-op except screen clear escape for HOME.)
- File ops (hosted stubs): `LOAD filename`, `SAVE filename`, `CATALOG` list directory. `NEW` clears program; `LIST [start],[end]` lists program; `RUN` executes program; `END` stops execution; `CLR` clears variables/for-stack/gosub stack/error handler.

## Control flow

- `IF expr THEN stmt-or-line [ELSE stmt-or-line]`: `expr` truthy if nonzero. THEN/ELSE may target a line number (`GOTO`-style) or an inline statement list.
- `GOTO n` jumps to line `n`; error if undefined.
- `GOSUB n` pushes return address then jumps to line `n`; `RETURN` pops; error if stack empty.
- `FOR V = start TO end [STEP s]` pushes loop control; `NEXT [V]` advances by `s` (default 1) and continues while (s>=0 ? V<=end : V>=end); otherwise pops. Variable name is optional in `NEXT` but matched if present.
- `ONERR GOTO n` sets error handler line; `RESUME` resumes at the line where the error occurred (once). Errors without handler print `?message IN LINE n`.

## Memory/PEEK/POKE/CALL

- `POKE addr, val` stores byte (val & 0xFF) in an emulated memory map.
- `PEEK(addr)` returns byte from map (default 0).
- `CALL addr` is a stub that evaluates the address expression but performs no action (placeholder for monitor calls).

## Expressions & operators

- Literals: decimal numbers (optional `.`, exponent `E`), and quoted strings.
- Variables/arrays: numeric or string; arrays use `()`. String functions require `$` suffix in names.
- Operators: unary `+` (no-op), unary `-`; binary `+ - * / ^ MOD`; comparisons `= <> < > <= >=`; logical `AND OR` (nonzero truthiness). Exponentiation is right-associative.
- Precedence (high → low): function calls/parentheses → unary ± → power → multiply/divide/mod → add/sub → relational → AND → OR.

## Built-in numeric functions

- `SIN(x)`, `COS(x)`, `TAN(x)`, `ATN(x)`
- `EXP(x)`, `LOG(x)`, `SQR(x)`
- `ABS(x)`, `INT(x)` (floor toward -∞), `SGN(x)` (-1/0/1)
- `RND(x)` (Applesoft semantics: RND(0) repeats seed? Here defers to Float40::rnd)
- `FRE(x)` returns fixed 32767 (placeholder free-memory)
- `PDL(x)` returns 0 (paddle not supported)
- `PEEK(x)` returns byte from emulated memory (see above)

## Built-in string & conversion functions

- `LEN(s)` length
- `VAL(s)` parses leading number else 0
- `ASC(s)` ASCII code of first char; ILLEGAL QUANTITY on empty string
- `CHR$(n)` single character for 0–255; error otherwise
- `LEFT$(s, n)` first `n` chars (clamped to [0,len])
- `RIGHT$(s, n)` last `n` chars (clamped)
- `MID$(s, start, len)` substring starting at 1-based `start`, length `len` (clamped; returns empty if start past end)
- `STR$(n)` renders number with leading space for positives

## Comments

- `REM ...` consumes rest of line (or until `:`) and is ignored.

## Error notes (runtime)

- Undefined line targets: `?UNDEF'D STATEMENT ERROR`
- READ past data: `?OUT OF DATA ERROR`
- Bad array subscript: `?BAD SUBSCRIPT ERROR`
- RETURN without GOSUB, NEXT without FOR, RESUME without error each raise runtime errors.
- I/O and file operations report `?FILE NOT FOUND ERROR` or `?I/O ERROR` as applicable.

## Emulation tips

- Preserve two-character variable significance and `$` suffix behavior; auto-dimension arrays to 10 on first use if not DIMed.
- Collect all DATA before RUN so READ works across control flow.
- Implement PRINT separators and newline rules faithfully; `?` alias for PRINT.
- Line-numbered storage with deterministic ordering; immediate statements run in isolated mode but share variable state unless `CLR`/`NEW` used.
- Error handler `ONERR`/`RESUME` should capture the failing line number, jump to handler, and allow a single RESUME to retry line.

# Copilot Instructions

## Big Picture

This repository implements an Applesoft II–compatible BASIC interpreter (`msbasic`) with the following goals:

- Faithful Applesoft semantics (DEF FN, DATA/READ/RESTORE, arrays, integer/string handling).
- Support for ProDOS-style commands and simple file I/O so scripts and examples behave like classic Applesoft.
- Portability across Linux, macOS and Windows with predictable behavior and minimal platform-specific code.
- Test-driven correctness: a comprehensive `.bas` test suite in `tests/` exercises language and runtime behavior.

## Reference Sources

- [Applesoft BASIC Commands — command & ProDOS command reference](../Research/applesoft_basic_commands.md)
- [Applesoft BASIC Functions — builtin function reference (SIN, RND, PEEK, etc.)](../Research/applesoft_basic_functions.md)
- [Applesoft BASIC Language Features — syntax, arrays, and control flow](../Research/applesoft_basic_language_features.md)
- [Applesoft BASIC Error Messages — error codes and handling guidance](../Research/applesoft_error_messages.md)
- [Applesoft PEEK/POKE/CALL Addresses — memory locations and ROM calls map](../Research/applesoft_peek_poke_call_addresses.md)

## Key Behaviors & Conventions

- Variable names: first two characters significant; `$` for strings, `%` for integers (clamped to 16‑bit).
- Arrays: auto‑DIM to size 10 per dimension if undeclared; subscripts are expressions; shared get/set path.
- DATA/READ/RESTORE: DATA collected pre‑run; READ is sequential across control flow; RESTORE optionally accepts a target line.
- Memory: `LOMEM`/`HIMEM` define valid range; `PEEK/POKE/WAIT` enforce bounds and raise “MEMORY RANGE ERROR” when out of range.
- WAIT: `WAIT addr,mask[,timeoutMs]` supports optional millisecond timeout; exits silently when elapsed.
- Output helpers: `TAB(n)`, `SPC(n)`, `POS()` (ANSI query on POSIX; Windows console via API, fallback to 0).
- Graphics: GR/HIRES/HPLOT/MOVE/ROTATE/SCALE/DRAW/XDRAW are implemented with an off‑screen buffer (no on‑screen rendering yet).
- Tracing & delays: `TRACE/NOTRACE` echo line numbers; `SPEED n` adds per‑statement delay (0–255 ms).

## Typical Change Recipes

- Add a statement/keyword:
  1.  Register in tokenizer keyword maps ([src/tokenizer.cpp](../src/tokenizer.cpp)).
  2.  Parse in `Parser::parseStatement()` and helpers ([src/parser.cpp](../src/parser.cpp)).
  3.  Implement runtime in a `Statement` subclass or `Interpreter` method ([src/interpreter.cpp](../src/interpreter.cpp)).
  4.  Add a `.bas` test under `tests/`.
- Add a built‑in function: implement in [src/functions.cpp](../src/functions.cpp) and parse in `FunctionCallExpr` paths.
- Extend variables/arrays: respect normalization in [src/variables.cpp](../src/variables.cpp); keep `%` coercion and auto‑DIM rules.

## Build, Run, Test

- Fast path: `cmake -S . -B build && cmake --build build`.
- VS Code task: `build-msbasic` builds target `msbasic`.
- Tests: `ctest --test-dir build --output-on-failure` executes all `.bas` programs in `tests/` via `msbasic`.
- Versioning: header generated from `src/version.h.in` → `build/generated/version.h` using `git describe` or override.
- REPL: `./build/msbasic` starts interactive mode; `./build/msbasic path/to/program.bas` runs a script.

## Project‑Specific Gotchas

- User functions `DEF FNx(a)=expr`: stored in `Variables`; calls like `FNx(expr)` substitute parameter then evaluate.
- Name normalization: `FN` names preserve 3 chars (`FNx`) significance; other names 2 chars (except `$`/`%`).
- Tests assume xterm‑like terminals for `POS`; Windows VT fallback is handled.
- Keep changes portable; guard OS‑specific code and avoid non‑ASCII.

## Hygiene

- Conventional commits (e.g., `feat: add WAIT timeout`).
- Prefer small, isolated patches matching existing style.
- Add `.bas` repros instead of C++ unit tests for language behavior.

Feedback: If any workflow or behavior above is unclear (e.g., adding a new command or graphics behavior), tell me which area to detail further and I’ll refine this guide.

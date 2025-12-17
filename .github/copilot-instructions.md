# Copilot Instructions

## Project Overview
- C++20 Applesoft II BASIC interpreter (`msbasic`) with custom 40-bit float implementation (`src/float40.*`).
- Core flow: tokenize → parse to AST → interpret; interactive shell uses same parser/interpreter.
- Variables are significant to the first two characters and may have `$` suffix for strings; arrays and user-defined functions are supported.
- DATA is pre-collected before program run; READ consumes in sequence; RESTORE resets the data pointer.

## Key Components
- Tokenizer: [src/tokenizer.cpp](../src/tokenizer.cpp) – converts BASIC source to tokens.
- Parser/AST: [src/parser.cpp](../src/parser.cpp) – parses statements/expressions, handles arrays, DEF FN, DATA/READ/RESTORE.
- Interpreter: [src/interpreter.cpp](../src/interpreter.cpp) – executes AST, manages DATA cursor and immediate mode.
- Variables & Functions: [src/variables.cpp](../src/variables.cpp) – storage for scalars, arrays, and user-defined functions.
- Runtime helpers: filesystem ([src/filesystem.cpp](../src/filesystem.cpp)), statements ([src/statements.cpp](../src/statements.cpp)), functions ([src/functions.cpp](../src/functions.cpp)).

## Build and Test
- Configure/build: `cmake -S . -B build && cmake --build build`.
- Tests: `ctest --test-dir build` executes `.bas` programs in `tests/` via the built `msbasic` binary.
- Generated version header: `build/generated/version.h` is produced from `src/version.h.in` at configure time.

## Coding Practices
- Keep code portable across Linux/macOS/Windows; avoid OS-specific APIs unless guarded.
- Prefer clear, small helpers over complex macros; follow existing style in neighboring code.
- Add brief comments only for non-obvious logic (e.g., parser/interpreter edge cases); keep files ASCII.

## Feature Behaviors to Preserve
- Variable name normalization to first two characters, string suffix `$` kept.
- Array subscripts are expressions; assignment and access use the same path.
- DATA list is collected before execution; RESTORE resets pointer; READ errors on exhaustion.
- DEF FN stores parsed expression; arguments resolved at call; function names share variable normalization rules.

## Testing Guidance
- Add `.bas` repros to `tests/`; keep them small and deterministic.
- Prefer exercising new syntax/behavior via `.bas` programs rather than C++ unit tests.

## When Changing the Interface
- Keep CLI behavior of `msbasic` stable; it loads files or enters interactive mode when no file is given.
- Update README and add tests when language surface or runtime behavior changes.

## Repository Hygiene
- When committing, unless instructed differently,  include all changes since the last commit 
- use a GitHub/Common Commit-style conventional message (e.g., `feat: add DATA handling`). 
- Pass the commit message explicitly via the commit command as a file parameter (e.g., `git commit -F message.txt`).

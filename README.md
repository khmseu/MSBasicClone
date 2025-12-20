# Applesoft BASIC Clone

A modern implementation of the Applesoft II BASIC interpreter written in C++20, targeting compatibility with the classic Apple II BASIC dialect.

## Features

### Implemented

- **Interactive Mode**: Classic "]" prompt with immediate command execution
- **Script Mode**: Execute BASIC programs from files
- **Variables**: 
  - Numeric and string variables (with $ suffix for strings)
  - Integer variables (with % suffix, 16-bit clamped)
  - First two characters significant for variable names
- **Arrays**: 
  - Multi-dimensional arrays with DIM
  - Auto-DIM to size 10 per dimension if undeclared
- **Operators**: Arithmetic (+, -, \*, /, ^, MOD), comparison (=, <>, <, >, <=, >=), logical (AND, OR, NOT)
- **Control Flow**:
  - IF/THEN/ELSE statements
  - FOR/NEXT loops (with STEP support)
  - WHILE/WEND loops
  - GOTO statements
  - GOSUB/RETURN subroutines
  - ON...GOTO/GOSUB statements
  - POP (clear GOSUB stack)
- **Data Handling**:
  - DATA/READ/RESTORE statements
  - RESTORE with optional line number
- **User-Defined Functions**: DEF FN with parameters
- **Error Handling**: ONERR GOTO and RESUME statements
- **Built-in Commands**:
  - NEW, RUN, LIST, END, STOP
  - LOAD, SAVE, CATALOG
  - PRINT (with semicolon, comma separators, TAB, SPC)
  - INPUT (with optional prompts)
  - GET (single character input)
  - LET (assignment, also works without LET keyword)
  - CLR (clear variables and control stacks)
  - REM (comments)
  - HOME (clear screen)
- **Math Functions**: SIN, COS, TAN, ATN, EXP, LOG, SQR, ABS, INT, SGN, RND, RANDOMIZE
- **String Functions**: LEFT$, RIGHT$, MID$, LEN, CHR$, ASC, VAL, STR$
- **Memory Commands**: 
  - PEEK/POKE with bounds checking
  - CALL (machine language call stub)
  - WAIT with optional timeout
  - LOMEM/HIMEM (memory bounds)
  - FRE (free memory)
  - USR (user machine code stub)
- **Screen Control**:
  - HTAB/VTAB (cursor positioning)
  - INVERSE/NORMAL/FLASH (text attributes)
  - TAB(), SPC(), POS() functions
- **Graphics Commands** (offscreen buffer, no rendering yet):
  - GR, HIRES, HGR, HGR2 (mode switching)
  - COLOR=, HCOLOR= (color selection)
  - PLOT, HPLOT (plotting points)
  - HLIN, VLIN (line drawing)
  - DRAW, XDRAW (shape drawing)
  - MOVE, ROTATE, SCALE (shape transformations)
  - SHLOAD (shape table loading)
  - SCRN() function (read pixel color)
- **Debugging**: TRACE/NOTRACE (line number display), SPEED (execution delay)
- **ProDOS Support**: PR#n, IN#n (I/O redirection stubs)
- **40-bit Floating Point**: Emulated Applesoft floating-point precision
- **Cross-Platform**: Compiles on Linux, macOS, and Windows

## Building

### Requirements

- CMake 3.20 or later
- GCC 13 or Clang 18 (or compatible)
- C++20 support

### Build Instructions

```bash
mkdir build
cd build
cmake ..
make
```

### Building with Clang

```bash
mkdir build
cd build
CXX=clang++ cmake ..
make
```

### Versioning

The build derives the version from `git describe --tags --dirty --always` (uses the output verbatim). If git data is unavailable, it falls back to `1.0.0` or a `-DVERSION_OVERRIDE=...` value passed to CMake. CPack picks up the same string via `CPACK_PACKAGE_VERSION`.

## Testing

Configure and build, then run the BASIC script tests via CTest (each `tests/*.bas` and `examples/*.bas` is executed and must exit cleanly):

```bash
cmake -S . -B build
cmake --build build
cd build
ctest
```

## Usage

### Interactive Mode

Run without arguments to start the interactive REPL:

```bash
./msbasic
```

You'll see the Applesoft-style prompt:

```text
APPLESOFT II BASIC CLONE <git describe output>
Compatible with Applesoft BASIC

]
```

### Script Mode

Run a BASIC program from a file:

```bash
./msbasic program.bas
```

## Example Programs

### Hello World

```basic
10 PRINT "HELLO, WORLD!"
20 END
```

### Fibonacci Sequence

```basic
10 REM FIBONACCI SEQUENCE
20 PRINT "FIBONACCI NUMBERS:"
30 LET A = 0
40 LET B = 1
50 FOR I = 1 TO 10
60 PRINT A
70 LET C = A + B
80 LET A = B
90 LET B = C
100 NEXT I
110 END
```

### Subroutine Example

```basic
10 PRINT "MAIN"
20 GOSUB 100
30 PRINT "BACK IN MAIN"
40 END
100 PRINT "IN SUBROUTINE"
110 RETURN
```

## Applesoft II Compatibility

This interpreter aims for source-code compatibility with Applesoft II BASIC as it appeared on the Apple II+, IIe, IIc, and IIGS. Key compatibility features:

- Only first 2 characters of variable names are significant (except for string and array variables)
- 40-bit floating-point precision
- Line numbers from 0-32767
- Classic BASIC syntax and semantics

## DOS/ProDOS Commands

File system commands are implemented with modern cross-platform file I/O:

- `LOAD "filename"` - Load a BASIC program
- `SAVE "filename"` - Save the current program
- `CATALOG` - List files in the current directory

## Development Status

This is a working interpreter with comprehensive Applesoft BASIC features implemented. The following areas are still in development or stubbed:

- **Graphics Rendering**: Graphics commands (GR, HIRES, PLOT, HPLOT, DRAW, XDRAW, etc.) maintain internal state and offscreen buffers but do not render to screen yet
- **Sound Commands**: BELL is implemented; other sound/audio commands not yet supported
- **Hardware-Specific Features**: Some Apple II-specific hardware commands are stubbed (PDL returns 0, USR returns 0, etc.)

## License

This project is open source. See LICENSE file for details.

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for bugs and feature requests.

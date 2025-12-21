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
  - NEW, RUN, LIST, END, STOP, CONT (continue after STOP)
  - LOAD, SAVE, CATALOG
  - DEL (delete line range)
  - RECALL/STORE (load/save arrays to files or tape)
  - TAPE (set/view current tape file)
  - SHLOAD (load shape tables from files or tape)
  - INPUT (with optional prompts)
  - GET (single character input)
  - LET (assignment, also works without LET keyword)
  - CLR (clear variables and control stacks)
  - REM (comments)
  - HOME (clear screen)
  - TEXT (switch to text mode)
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
- **Graphics Commands**:
  - GR, HIRES, HGR, HGR2 (mode switching)
  - COLOR=, HCOLOR= (color selection)
  - PLOT, HPLOT (plotting points)
  - HLIN, VLIN (line drawing)
  - DRAW, XDRAW (shape drawing)
  - MOVE, ROTATE, SCALE (shape transformations)
  - SHLOAD (shape table loading)
  - SCRN() function (read pixel color)
  - Window rendering with Raylib (when available and display present)
  - Graceful fallback to off-screen buffer when no display
- **Debugging**: TRACE/NOTRACE (line number display), SPEED (execution delay)
- **ProDOS Support**: 
  - PR#n, IN#n (I/O redirection)
  - PR#0 (40-column text mode)
  - PR#3 (80-column text mode)
- **40-bit Floating Point**: Emulated Applesoft floating-point precision
- **Cross-Platform**: Compiles on Linux, macOS, and Windows

## Building

### Requirements

- CMake 3.20 or later
- GCC 13 or Clang 18 (or compatible)
- C++20 support
- Raylib 5.0+ (optional, for graphics rendering)
- X11 development libraries (optional, for graphics on Linux)

### Build Instructions

```bash
mkdir build
cd build
cmake ..
make
```

#### Automatic Raylib Build

By default, the build system will automatically download and build Raylib from source if it's not found on your system. This requires:

**Linux:**
```bash
# Install X11 development libraries first
sudo apt-get install libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libgl1-mesa-dev
```

**macOS:**
No additional dependencies needed - Raylib will build automatically.

**Windows:**
Raylib will build automatically with MinGW or MSVC.

To disable automatic Raylib building:
```bash
cmake -DAUTO_BUILD_RAYLIB=OFF ..
```

#### Manual Raylib Installation (Alternative)

If you prefer to install Raylib manually:

**Ubuntu/Debian:**
```bash
# Install dependencies
sudo apt-get install libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libgl1-mesa-dev

# Clone and build Raylib
git clone --depth 1 --branch 5.0 https://github.com/raysan5/raylib.git
cd raylib
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_EXAMPLES=OFF
cd build
make
sudo make install
```

**macOS:**
```bash
brew install raylib
```

The build system will detect Raylib whether installed manually or built automatically. Without Raylib, graphics commands will use off-screen buffers only.

### Font Auto-Fetching

The build system automatically downloads the Ultimate Apple II Font during CMake configuration:

- **Font**: `PrintChar21.ttf` - The Ultimate Apple II Font (7×8 pixel characters)
- **Charset Map**: `apple2-charset.html` - Apple II character set reference
- **Source**: https://www.kreativekorp.com/software/fonts/apple2/

Downloaded files are cached in `assets/fonts/` and reused across builds. If the download fails (e.g., due to network restrictions), the build continues with warnings, and Raylib's default font is used as a fallback.

For manual font installation or troubleshooting, see the [Font Integration](docs/features.md#font-integration) section in the features documentation.

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

### Graphics Mode Options

The interpreter supports two modes for graphics handling:

```bash
# Run with graphics rendering enabled (default)
./msbasic --graphics program.bas

# Run in no-graphics mode (graphics commands will error)
./msbasic --no-graphics program.bas

# Set window scale factor (1-10, default is 2)
./msbasic --scale 3 program.bas

# Set default tape file for STORE/RECALL/SHLOAD
./msbasic --tape /tmp/mytape.tap program.bas

# Set tape change hotkey (default: ESC-T)
./msbasic --tape-hotkey "^T" program.bas
```

### Tape Emulation

The interpreter supports cassette tape emulation for array and shape table persistence:

**Using command-line:**
```bash
./msbasic --tape /tmp/mytape.tap program.bas
```

**Using BASIC commands:**
```basic
10 TAPE "/tmp/mytape.tap"    REM Set current tape file
20 TAPE                       REM Display current tape
30 DIM A(5)
40 FOR I = 0 TO 5: A(I) = I * 10: NEXT I
50 STORE A                    REM Save array to tape
60 FOR I = 0 TO 5: A(I) = 0: NEXT I
70 RECALL A                   REM Load array from tape
```

**Tape Features:**
- Sequential record format with SIZE|DATA headers
- Supports arrays (numeric and string) via STORE/RECALL
- Supports shape tables via SHLOAD
- Falls back to file-based storage when no tape loaded
- Use `TAPE` with no argument to view current tape
- OS-native file selector support (zenity/kdialog on Linux, osascript on macOS, COM on Windows)

**Note:** Tape access is sequential - each RECALL/SHLOAD opens from the beginning, matching authentic cassette tape behavior.

**Graphics Mode** (default): When graphics are enabled, the interpreter will attempt to open a window for rendering graphics commands (GR, HGR, HPLOT, etc.). If no display is available (e.g., in a headless environment), graphics will fall back to off-screen buffer mode only.

**No-Graphics Mode**: All graphics commands (GR, HGR, PLOT, HPLOT, etc.) will raise a "GRAPHICS NOT ENABLED ERROR" when executed. This is useful for running programs in terminal-only environments or for testing.

### Text Mode Commands

The interpreter supports 40-column and 80-column text modes via the PR# command:

- `PR#0` - Switch to 40-column text mode (default)
- `PR#3` - Switch to 80-column text mode

These commands are compatible with Applesoft BASIC conventions for peripheral slot I/O redirection.

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

This is a working interpreter with comprehensive Applesoft BASIC features implemented. The following areas are still in development:

- **Graphics Rendering**: Graphics commands are implemented with Raylib support for window rendering. When Raylib is not available or no display is present, graphics fall back to off-screen buffer mode.
- **Font Rendering**: The Ultimate Apple II Font integration is planned for future releases for authentic text rendering in graphics mode.
- **Sound Commands**: BELL is implemented; other sound/audio commands not yet supported
- **Hardware-Specific Features**: Some Apple II-specific hardware commands are stubbed (PDL returns 0, USR returns 0, etc.)

## License

This project is open source. See LICENSE file for details.

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for bugs and feature requests.

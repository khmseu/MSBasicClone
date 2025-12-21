# MSBasic Architecture Documentation

## Overview

MSBasic is a modern implementation of the Applesoft II BASIC interpreter written in C++20. The project aims to provide faithful Applesoft BASIC semantics while being portable across Linux, macOS, and Windows platforms. It supports both interactive REPL mode and script execution, with optional graphics rendering via Raylib.

### Design Goals

1. **Applesoft II Compatibility**: Faithful reproduction of Applesoft BASIC behavior including:
   - Variable name normalization (first 2 characters significant)
   - 40-bit floating-point arithmetic
   - Classic control flow and data structures
   - Memory-mapped I/O simulation (PEEK/POKE/CALL)

2. **Cross-Platform Portability**: Clean separation between platform-specific and portable code, with consistent behavior across operating systems.

3. **Test-Driven Development**: Comprehensive `.bas` test suite ensures correctness and prevents regressions.

4. **Modern Implementation**: Leverages C++20 features while maintaining clarity and performance.

## System Architecture

MSBasic follows a classic interpreter architecture with four main phases:

```text
Source Code → [Tokenizer] → Tokens → [Parser] → AST → [Interpreter] → Execution
                                                           ↓
                                                    [Variables/Memory]
                                                           ↓
                                                    [Graphics Subsystem]
```

### High-Level Components

1. **Tokenizer** (`src/tokenizer.cpp/h`): Lexical analysis
2. **Parser** (`src/parser.cpp/h`): Syntax analysis and AST construction
3. **Interpreter** (`src/interpreter.cpp/h`): Runtime execution engine
4. **Variables** (`src/variables.cpp/h`): Variable and array storage
5. **Functions** (`src/functions.cpp/h`): Built-in function implementations
6. **Statements** (`src/statements.cpp/h`): Statement execution classes
7. **Graphics** (`src/graphics.cpp/h`, `src/graphics_renderer.cpp/h`): Graphics subsystem
8. **Float40** (`src/float40.cpp/h`): 40-bit floating-point emulation
9. **Interactive** (`src/interactive.cpp/h`): REPL mode implementation
10. **Filesystem** (`src/filesystem.cpp/h`): File I/O operations
11. **Tape Manager** (`src/tape_manager.cpp/h`): Cassette tape emulation

## Component Details

### 1. Tokenizer

**Purpose**: Convert raw text into a stream of tokens for parsing.

**Key Responsibilities**:

- Lexical analysis of BASIC source code
- Keyword recognition (PRINT, IF, FOR, etc.)
- Number and string literal parsing
- Operator recognition (+, -, \*, /, ^, MOD, etc.)
- Identifier extraction with special handling for $, %, and FN prefixes

**Design Notes**:

- Single-pass tokenization
- Line-aware for error reporting
- Supports both immediate commands and program lines
- Handles multi-statement lines (colon `:` separator)

**Key Classes/Functions**:

- `Tokenizer::tokenize(const std::string& line)`: Main entry point
- `Tokenizer::isKeyword()`: Keyword lookup
- Token type enumeration in `types.h`

### 2. Parser

**Purpose**: Transform tokens into an Abstract Syntax Tree (AST).

**Key Responsibilities**:

- Recursive descent parsing
- Expression parsing with proper operator precedence
- Statement parsing (PRINT, IF, FOR, GOSUB, etc.)
- Error detection and reporting

**Expression Parsing Hierarchy** (highest to lowest precedence):

1. Primary expressions (literals, variables, function calls, parentheses)
2. Power operator (^)
3. Unary operators (+, -, NOT)
4. Multiplicative operators (\*, /, MOD)
5. Additive operators (+, -)
6. Relational operators (<, >, <=, >=, =, <>)
7. NOT operator
8. AND operator
9. OR operator

**Statement Parsing**:

- Each statement type has a dedicated parser method (`parseIf()`, `parseFor()`, etc.)
- Supports multi-statement lines via `parseStatement()` loop
- Handles both numbered program lines and immediate mode commands

**Key Classes**:

- `Expression`: Base class for all expression AST nodes
- `Statement`: Base class for all statement AST nodes
- `Parser`: Main parser class with recursive descent methods

### 3. Interpreter

**Purpose**: Execute the parsed AST and manage runtime state.

**Key Responsibilities**:

- Program execution control (run, stop, continue)
- Variable and array management via `Variables` class
- Control flow (GOTO, GOSUB/RETURN, FOR/NEXT, WHILE/WEND)
- Data statement processing (DATA/READ/RESTORE)
- Error handling (ONERR GOTO, RESUME)
- I/O operations (PRINT, INPUT, GET)
- Graphics mode coordination
- Memory simulation (PEEK/POKE/CALL/WAIT)

**Execution Model**:

- Program stored as map of `LineNumber` → `ProgramLine`
- Program counter (`programCounter_`) iterates through sorted lines
- Jump flag (`jumped_`) prevents auto-increment after GOTO/GOSUB
- Stack-based GOSUB return tracking
- Nested FOR loop management with stack

**State Management**:

- **Running state**: `running_`, `paused_`, `immediate_`
- **Execution position**: `currentLine_`, `programCounter_`
- **Control stacks**: `gosubStack_`, `forStack_`, `whileStack_`
- **Data handling**: `dataValues_`, `dataPointer_`, `dataOffsets_`
- **Error handling**: `errorHandlerLine_`, `lastError_`, `errorLine_`
- **Output state**: `outputColumn_`, `outputRow_`, text attributes
- **Memory bounds**: `lomem_`, `himem_`

**Key Methods**:

- `run()`: Execute program from start or current position
- `executeImmediate()`: Execute single command in immediate mode
- `gotoLine()`, `gosub()`, `returnFromGosub()`: Control flow
- `addLine()`, `deleteLine()`: Program editing
- `loadProgram()`, `saveProgram()`: File I/O

### 4. Variables

**Purpose**: Store and manage program variables, arrays, and user-defined functions.

**Key Responsibilities**:

- Variable storage (numeric, string, integer)
- Multi-dimensional array support
- Auto-dimensioning arrays to size 10
- Variable name normalization (Applesoft convention)
- User-defined function (DEF FN) storage

**Variable Name Normalization**:

- First 2 characters significant (case-insensitive)
- Exception: FN names preserve 3-4 characters (FN prefix + 2 chars)
- String suffix `$` preserved
- Integer suffix `%` preserved

**Array Support**:

- Multi-dimensional arrays with arbitrary dimension count
- Auto-DIM to size 10 per dimension if undeclared
- Bounds checking with "BAD SUBSCRIPT" error
- Sparse storage using `std::map<std::vector<int>, Value>`

**User-Defined Functions**:

- Stored with parameter name and expression AST
- Function calls substitute parameter value into expression
- Naming follows FN conventions (FN prefix + 2 chars)

**Key Classes/Methods**:

- `setVariable()`, `getVariable()`: Simple variable access
- `dimArray()`, `setArrayElement()`, `getArrayElement()`: Array operations
- `defineFunction()`, `getFunction()`: User function management
- `normalizeName()`: Applesoft-compatible name normalization

### 5. Functions

**Purpose**: Implement all built-in functions (math, string, system).

**Function Categories**:

**Mathematical Functions**:

- Trigonometric: SIN, COS, TAN, ATN
- Exponential/Logarithmic: EXP, LOG, SQR
- Numeric: ABS, INT, SGN, RND

**String Functions**:

- Extraction: LEFT$, RIGHT$, MID$
- Conversion: CHR$, ASC, STR$, VAL
- Information: LEN

**System Functions**:

- Memory: PEEK, FRE
- Screen: POS, SCRN
- Hardware: PDL, USR (stubs)
- Formatting: TAB, SPC

**Implementation**:

- Function evaluation occurs during expression evaluation
- Functions are parsed as special expression nodes
- Built-in functions have dedicated evaluator methods
- User-defined functions (FN) stored in Variables class

### 6. Statements

**Purpose**: Define executable statement types and their behavior.

**Statement Types**:

**Control Flow**:

- Conditional: IF/THEN/ELSE
- Loops: FOR/NEXT, WHILE/WEND
- Jumps: GOTO, GOSUB/RETURN, ON...GOTO/GOSUB
- Stack manipulation: POP

**I/O**:

- Output: PRINT, HTAB, VTAB, HOME
- Input: INPUT, GET
- Text modes: INVERSE, NORMAL, FLASH

**Data Management**:

- Variables: LET (implicit assignment)
- Arrays: DIM
- Data: DATA, READ, RESTORE

**Graphics**:

- Mode switching: GR, HIRES, HGR, HGR2, TEXT
- Drawing: PLOT, HPLOT, HLIN, VLIN
- Shapes: DRAW, XDRAW, MOVE, ROTATE, SCALE, SHLOAD
- Color: COLOR=, HCOLOR=

**Program Control**:

- Execution: RUN, END, STOP, CONT
- Editing: NEW, LIST, DEL
- Files: LOAD, SAVE, CATALOG
- Arrays: RECALL, STORE

**Memory/System**:

- Memory: POKE, CALL, WAIT
- Bounds: LOMEM, HIMEM
- Debugging: TRACE, NOTRACE, SPEED
- Error handling: ONERR, RESUME
- Devices: PR#, IN#
- Tape: TAPE

**ProDOS Commands**:

- Files: CREATE, DELETE, RENAME, LOCK, UNLOCK
- I/O: OPEN, CLOSE, READ, WRITE, APPEND, POSITION, FLUSH
- Execution: EXEC, CHAIN, - (dash)
- Directory: PREFIX, CAT

**Implementation**:

- Each statement type is a class derived from `Statement`
- `execute()` method performs the statement's action
- Statements access interpreter state via passed `Interpreter*`

### 7. Graphics Subsystem

**Purpose**: Provide Apple II-compatible graphics rendering.

**Architecture**:

- Two-tiered design: off-screen buffer + optional window rendering
- Off-screen buffer always maintained for SCRN() queries
- Window rendering via Raylib (optional, graceful fallback)

**Graphics Modes**:

- **Text Mode**: 40-column or 80-column text
- **Low-Res Mode (GR)**: 40×40 pixels, 16 colors
- **High-Res Mode (HGR/HGR2)**: 280×192 pixels, 6 colors

**Rendering Pipeline**:

```text
Graphics Command → Graphics Buffer Update → Raylib Window Render
```

**Key Components**:

- `Graphics` (singleton): Command interface and state management
- `GraphicsRenderer`: Raylib rendering backend
- `GraphicsConfig`: Configuration (mode, scale, text columns)

**Shape Tables**:

- Binary format with vector data
- ROTATE and SCALE transformations
- DRAW (normal) and XDRAW (XOR) modes
- SHLOAD from files or tape

**Text Rendering**:

- Ultimate Apple II Font (PrintChar21.ttf)
- 7×8 pixel character cells
- 40-column: 280 pixels wide
- 80-column: Horizontal compression to fit 280 pixels

### 8. Float40

**Purpose**: Emulate Applesoft's 40-bit floating-point format.

**Format**:

- 5 bytes total
- 1 byte exponent (excess-128)
- 4 bytes mantissa (normalized)
- Sign bit in high bit of mantissa

**Operations**:

- Addition, subtraction, multiplication, division
- Conversion to/from IEEE 754 double
- Comparison operations

**Design Notes**:

- Preserves Applesoft numerical behavior
- Handles edge cases (zero, infinity, NaN)
- Used internally for all numeric computations

### 9. Interactive Mode

**Purpose**: Provide REPL (Read-Eval-Print Loop) interface.

**Features**:

- Classic `]` prompt
- Immediate command execution
- Program line editing (numbered lines)
- Command history
- Tape change hotkey support

**Command Types**:

- **Immediate commands**: Execute immediately (PRINT, LIST, RUN, etc.)
- **Program lines**: Stored with line numbers for later execution
- **Editing**: Line replacement or deletion

**Implementation**:

- `InteractiveMode` class manages REPL loop
- Uses `Interpreter` for command execution
- Handles line input and parsing
- Supports both Unix and Windows console I/O

### 10. Filesystem

**Purpose**: Provide file I/O operations for BASIC programs and data.

**Supported Operations**:

- Program files: LOAD, SAVE
- Directory listing: CATALOG, CAT
- Binary files: BLOAD, BSAVE, BRUN
- Array persistence: RECALL, STORE
- ProDOS operations: CREATE, DELETE, RENAME, OPEN, CLOSE, etc.

**File Formats**:

- Text format for BASIC programs (line-numbered text)
- Binary format for arrays (SIZE header + data)
- Binary format for shape tables
- Sequential tape format for STORE/RECALL/SHLOAD

**Cross-Platform**:

- Uses C++ standard library (std::filesystem, std::fstream)
- Path normalization for different OSes
- Native file dialogs via platform APIs (zenity, kdialog, osascript, COM)

### 11. Tape Manager

**Purpose**: Emulate cassette tape for data persistence.

**Features**:

- Sequential record format
- Arrays: STORE/RECALL
- Shape tables: SHLOAD
- Position tracking across operations
- File fallback when no tape loaded

**Tape Format**:

- Header: SIZE (4 bytes, little-endian)
- Data: Variable-length record
- Sequential access (rewind via TAPE command)

**Use Cases**:

- Save/load multiple arrays sequentially
- Shape table distribution
- Data persistence without explicit filenames

## Data Flow

### Program Execution Flow

1. **Load Phase**:
   - Read source file or accept interactive input
   - Tokenize each line
   - Parse tokens into statements
   - Store in program map (LineNumber → ProgramLine)

2. **Data Collection Phase** (pre-run):
   - Scan all DATA statements
   - Collect values into `dataValues_` vector
   - Build line → offset mapping for RESTORE

3. **Execution Phase**:
   - Initialize program counter to first line
   - Loop through program lines:
     - Apply SPEED delay if configured
     - Trace line number if TRACE enabled
     - Execute each statement in line
     - Update program counter (unless jumped)
     - Check for END or STOP
   - Handle errors via ONERR if configured

4. **Control Flow**:
   - GOTO: Jump to target line
   - GOSUB: Push return address, jump to subroutine
   - RETURN: Pop return address, resume execution
   - FOR/NEXT: Track loop state, auto-increment, check bounds
   - WHILE/WEND: Evaluate condition, loop or exit

### Expression Evaluation Flow

1. Parse expression into AST (recursive descent)
2. Evaluate AST (depth-first traversal):
   - Literals: Return value directly
   - Variables: Lookup in Variables class
   - Arrays: Evaluate subscripts, lookup element
   - Functions: Evaluate arguments, call function
   - Operators: Evaluate operands, apply operation
3. Type coercion as needed (string/number)
4. Return final Value

### Graphics Rendering Flow

1. **Command Reception**: Graphics command executed (HPLOT, etc.)
2. **Buffer Update**: Off-screen buffer modified
3. **Window Update** (if Raylib available):
   - Clear window
   - Iterate buffer pixels
   - Draw to Raylib texture
   - Display texture scaled
   - Poll events
4. **No-Graphics Fallback**: Command raises error if mode disabled

## Memory Model

### Variable Storage

- Variables stored in `std::map<std::string, Value>` (normalized names)
- Arrays stored separately in `std::map<std::string, ArrayInfo>`
- User functions stored in `std::map<std::string, FunctionInfo>`

### Program Storage

- Program stored as `std::map<LineNumber, ProgramLine>`
- Sorted by line number for sequential execution
- Each line contains: line number, source text, tokens, parsed statements

### Simulated Memory

MSBasic simulates Apple II memory locations for compatibility:

**Key Memory Locations**:

- **$0069-$006A (105-106)**: LOMEM pointer
- **$0073-$0074 (115-116)**: HIMEM pointer
- **$00D8 (216)**: Error handler control
- **$00DA-$00DB (218-219)**: Error line number
- **$00DE (222)**: Error code
- **$C000 (49152/-16384)**: Last key pressed
- **$C010 (49168/-16368)**: Clear keyboard strobe
- **$C050-$C057**: Graphics mode soft switches

**PEEK/POKE/CALL Implementation**:

- Simulated memory array (64KB)
- Bounds checking against LOMEM/HIMEM
- Special handling for control addresses
- CALL is currently a no-op stub

### Data Segment

- DATA values stored in `std::vector<Value>`
- Read pointer tracks current position
- RESTORE can seek to specific line's data offset

## Build System

### CMake Configuration

**Key Features**:

- C++20 requirement
- Optional Raylib auto-build via FetchContent
- Font auto-fetch from kreativekorp.com
- Cross-platform support (Linux, macOS, Windows)
- CTest integration for `.bas` test suite

**Version Management**:

- Git-based versioning: `git describe --tags --dirty --always`
- Fallback to default or override
- Embedded in `version.h` header

**Dependencies**:

- Raylib 5.0+ (optional, auto-built)
- X11 libraries (Linux only, for Raylib)
- Standard C++20 toolchain

**Build Targets**:

- `msbasic`: Main executable
- CTest tests: Each `.bas` file in `tests/` and `examples/`

### Font Integration

**Automatic Font Fetching**:

- CMake module: `cmake/FetchFont.cmake`
- Downloads Ultimate Apple II Font at configure time
- Caches in `assets/fonts/`
- Idempotent (skips if already present)
- Graceful fallback on failure

**Font Files**:

- `PrintChar21.ttf`: TrueType font (~50KB)
- `apple2-charset.html`: Character set reference
- `FreeLicense.txt`: License file

**CI/CD Integration**:

- GitHub Actions cache for fonts
- Cache key based on `FetchFont.cmake` hash
- Speeds up CI builds

## Testing Strategy

### Test Suite

**Location**: `tests/` and `examples/` directories

**Test Types**:

1. **Language Feature Tests**: Arrays, loops, conditionals, functions, etc.
2. **Graphics Tests**: Drawing, modes, shapes
3. **System Tests**: PEEK/POKE, memory, errors
4. **File I/O Tests**: LOAD/SAVE, arrays, ProDOS commands
5. **Example Programs**: Demonstrations and edge cases

**Execution**:

- CTest runs each `.bas` file
- Success: Program exits with code 0
- Failure: Non-zero exit or runtime error

**Coverage**:

- 75+ test programs
- ~98% feature coverage
- Core language features: 100%
- Graphics: 100%
- ProDOS: 100%

### Manual Testing

- Interactive mode: REPL correctness
- Graphics rendering: Visual inspection
- Cross-platform: Linux, macOS, Windows builds
- No-graphics mode: Headless operation

## Cross-Platform Considerations

### Platform-Specific Code

**Conditional Compilation**:

- `PLATFORM_LINUX`, `PLATFORM_MACOS`, `PLATFORM_WINDOWS` defines
- Guards for Windows vs. POSIX APIs

**Platform Differences**:

**Windows**:

- Virtual Terminal mode for ANSI support
- Console API for cursor positioning
- COM file dialogs

**Linux/macOS (POSIX)**:

- ANSI escape sequences
- X11 for graphics (Linux)
- Cocoa for graphics (macOS)
- zenity/kdialog/osascript for file dialogs

### Portability Techniques

1. **Standard Library First**: Use C++ standard library when possible
2. **Platform Guards**: `#ifdef` blocks for OS-specific code
3. **Abstraction Layers**: Graphics subsystem hides platform details
4. **Graceful Degradation**: Fallbacks for missing features
5. **CI/CD Testing**: GitHub Actions tests Linux, macOS, Windows

## Error Handling

### Error Types

**Syntax Errors**:

- Detected during tokenization/parsing
- Reported immediately
- Prevent program execution

**Runtime Errors**:

- Type mismatch
- Division by zero
- Out of memory
- Bad subscript
- Undefined statement/function
- Graphics not enabled
- File I/O errors

**ProDOS Error Codes**:

- Range: 2-21
- Stored in memory location 222
- Examples: PATH NOT FOUND, DISK FULL, FILE LOCKED

### Error Handling Flow

1. **No Handler**: Print error message, stop execution
2. **ONERR GOTO n**: Jump to error handler line
3. **Error State**: Store error code, error line
4. **RESUME**: Continue execution from error line or next line

### Error Recovery

- ONERR GOTO 0: Disable error handler
- POKE 216,0: Restore normal error handling
- CLR: Clear all state including error handler

## Performance Considerations

### Optimizations

1. **Tokenization**: Single-pass, minimal allocations
2. **Variable Lookup**: Hash map (O(1) average)
3. **Array Storage**: Sparse map (memory-efficient)
4. **Program Storage**: Sorted map (O(log n) line lookup)
5. **Graphics Buffer**: Direct pixel access, minimal copying

### Bottlenecks

1. **Expression Evaluation**: Recursive AST traversal
2. **Graphics Rendering**: Pixel-by-pixel updates
3. **String Operations**: Frequent allocations
4. **Float40 Arithmetic**: Non-native format conversion

### Scalability

- Handles programs with thousands of lines
- Arrays limited by available memory
- Graphics: Fixed 280×192 buffer (minimal overhead)

## Future Enhancements

### Potential Improvements

1. **JIT Compilation**: Translate AST to native code
2. **Bytecode VM**: Intermediate representation
3. **Optimizing Parser**: Constant folding, dead code elimination
4. **Native Float**: Option to use IEEE 754 instead of Float40
5. **Sound Support**: BELL enhancement, tone generation
6. **Network I/O**: TCP/IP extensions
7. **Debugger**: Breakpoints, watch expressions, step-through
8. **More Graphics**: Enhanced shape tables, sprites
9. **Plugin System**: Loadable extensions
10. **Documentation**: Generate API docs with Doxygen

### Backward Compatibility

- Maintain Applesoft semantics
- Support legacy programs
- Optional modern extensions (flags/pragmas)

## References

- [Implementation Features](features.md)
- [README](../README.md)

Additional reference materials for Applesoft BASIC (commands, functions, language features, error messages, and memory addresses) are available in the Research folder but are not included in the API documentation.

## Glossary

- **AST**: Abstract Syntax Tree - hierarchical representation of program structure
- **BASIC**: Beginner's All-purpose Symbolic Instruction Code
- **Float40**: 40-bit floating-point format used by Applesoft BASIC
- **HIMEM**: High memory boundary for BASIC program/variables
- **LOMEM**: Low memory boundary for BASIC program/variables
- **PEEK/POKE**: Read/write memory directly by address
- **ProDOS**: Apple II disk operating system with extended commands
- **REPL**: Read-Eval-Print Loop - interactive command interface
- **Tokenizer**: Component that converts text to tokens
- **Parser**: Component that converts tokens to AST
- **Interpreter**: Component that executes AST

---

Last Updated: 2025-12-21

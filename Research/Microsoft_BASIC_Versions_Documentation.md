# Comprehensive History of Microsoft BASIC Versions

**Research Date:** December 17, 2025

---

## Table of Contents

1. [Early Era (1975-1980s)](#early-era-1975-1980s)
2. [Professional Era (1985-1998)](#professional-era-1985-1998)
3. [Modern Era (2002+)](#modern-era-2002)
4. [Apple-Specific BASIC Variants](#apple-specific-basic-variants)
5. [Summary Timeline](#summary-timeline)
6. [Sources](#sources)

---

## Early Era (1975-1980s)

### 1. Altair BASIC (1975)

**Version Range:** 2.0 through 5.0 (1975-1978)

**Release Date:** July 1, 1975

**Key Capabilities:**

- **4K BASIC:** Minimal version (4KB RAM total, ~790 bytes free for user code)
  - Mathematical functions: `SQR`, `RND`, `SIN`, `ABS`, `INT`
  - Control structures: `IF...THEN`, `FOR...NEXT`, `GOSUB`/`RETURN`
  - Single-precision 32-bit floating point
  - Variable names: single letter or letter+digit (max 286 variables)
  - No string variables

- **8K BASIC:** Added string manipulation
  - String variables with `$` suffix
  - String functions: `MID$`, `LEN$`, concatenation
  - More mathematical functions
  - Boolean operators

- **Extended BASIC:** Advanced features
  - Double-precision 64-bit variables (`#` suffix)
  - `IF...THEN...ELSE` structures
  - User-defined functions
  - Advanced program editing commands
  - Descriptive error messages

- **Disk BASIC:** Extended disk commands for raw I/O

**Platforms:** Altair 8800, S-100 bus computers

**Pricing (1975):** $150 (4K), $200 (8K), $350 (Extended)

---

### 2. IBM Cassette BASIC (1981)

**Version Range:** C1.00, C1.10, C1.20

**Key Capabilities:**

- 32 KB ROM-based interpreter
- Cassette tape storage (no floppy disk support)
- Integrated into original IBM PC and XT
- Graphics capabilities for IBM PC hardware
- Sound support via PC speaker
- Memory-constrained (required 16-64 KB RAM)
- Default user interface when PC booted without floppy disk

**Platforms:** IBM PC, IBM XT, early PS/2 models

---

### 3. IBM Disk BASIC (1981)

**Version Range:** Based on DOS versions 1.0 through 3.30

**Key Capabilities:**

- Built on Cassette BASIC ROM (32 KB)
- Floppy disk and cassette tape support
- Errata corrections from Cassette BASIC
- Serial-port support
- "Light" version for 48 KB memory PCs
- ~23 KB free for user code

**Platforms:** IBM PC, IBM PC compatibles

---

### 4. IBM Advanced BASIC (BASICA) (1981)

**Version Range:** 1.00 through 3.30 (matching DOS versions)

**Key Capabilities:**

- Diskette file access and storage
- Sound generation via PC speaker (monophonic)
- Graphics functions: pixel setting/clearing, lines, circles, color setting
- Event handling for communications and joystick input
- `PAINT` command for flood filling
- `PLAY` statement for music macro language
- `SOUND` statement for frequency-based sounds
- Graphics improvements in v2.x and up

**Platforms:** IBM PC, IBM PC DOS

**Note:** Not compatible with non-IBM computers or later IBM models without Cassette BASIC ROM

---

### 5. IBM PCjr Cartridge BASIC (1984)

**Key Capabilities:**

- ROM cartridge format
- Superset of Advanced BASIC
- Support for PCjr's enhanced graphics modes and sound capabilities
- Memory limitation: First 128 KB only (no expansion RAM support)

**Platforms:** IBM PCjr only

---

### 6. GW-BASIC (1983-1988)

**Version Range:** 1.0 through 3.23

**Release Date:** 1983 (Version 1.0 with Compaq DOS 1.13)

**Key Capabilities:**

- Functionally identical to BASICA but standalone executable
- No dependence on Cassette BASIC ROM
- Command-line IDE based on Dartmouth BASIC
- Cursor-based line editor with function key shortcuts

**Language Features:**

- `WHILE...WEND` loops
- 40-character variable names
- `OPTION BASE` statement
- Dynamic string space allocation
- `LINE INPUT` for field-separator handling
- `CALL` statement for machine language
- `CHAIN` and `MERGE` commands
- Program save in tokenized binary or ASCII text

**Graphics Capabilities:**

- CGA graphics support (v1.0 and later)
- EGA graphics support (v3.20)
- Hercules monochrome graphics (HBASIC variant)
- No VGA support

**Music/Sound:**

- `PLAY` statement with music macro language
- `SOUND` statement for frequency control
- Tandy 1000 version: up to 3 sound channels

**Device Support:**

- Joystick and light pen input
- Serial port (COM port) access
- Disk file I/O
- Event trapping for ports

**Platforms:** MS-DOS on IBM PC compatibles

---

### 7. MBASIC (CP/M) (1976+)

**Version Range:** Based on BASIC-80 5.x

**Key Capabilities:**

- Stripped-down BASIC-80 version for CP/M
- Hardware-neutral functions only
- Compiler: BASCOM or BASCOM32

**Platforms:** CP/M-80 (8080, 8085, Z80), ISIS-II, TEKDOS

---

### 8. BASIC-86 (1979-1980)

**Key Capabilities:**

- 8-bit FAT file system support (early implementation)
- 16-bit system support
- Standalone disk-based language system

**Platforms:** Seattle Computer Products S-100 bus 8086, Intel 8086 SBC-86/12

---

### 9. TRS-80 BASIC (1977-1983)

**Versions:** Level I, Level II, Level III, Model 4 versions

**Level II & III Capabilities:**

- Level II: Port of BASIC-80 with Level I graphics (SET, PSET)
- Level III: Descriptive error messages, user-defined functions (DEF FN)
- Model 4 version: BASIC-80 5.x core
  - 40-character variable names
  - `DEF FN` support
  - TRSDOS 6 system function access via `SYSTEM` keyword
  - Line-oriented editing environment

**Platforms:** Tandy/Radio-Shack TRS-80, TRS-80 Model 4

---

### 10. 6502 BASIC (1976+)

**Versions:** Various implementations with vendor extensions

**Base Capabilities:**

- 8K or 9K versions
- 40-bit floating-point support (most common)
- Dynamic string allocation
- User-defined functions from Extended BASIC
- Descriptive error messages
- Line crunching support
- `GET` statement for keyboard input
- Single and double-precision floating point support (vendor-dependent)

**Commodore Extensions:**

- **Disk Commands:** `DIRECTORY`, `DSAVE`, `DLOAD`, `BACKUP`, `HEADER`, `SCRATCH`, `COLLECT`
- **Graphics:** `CIRCLE`, `DRAW`, `BOX`, `COLOR`, `PAINT`, `SSHAPE`, `GSHAPE`
- **Sprites:** Commands on C128
- **Sound:** `VOL`, `SOUND`, music commands with ADSR and SID filter
- **Structured Programming:** `IF...THEN...ELSE`, `DO...LOOP...WHILE/UNTIL`
- **Extended I/O:** `JOY`, function keys
- **Debugging:** `STOP`, `CONT`, `TRON`, `TROFF`, `RESUME`, `RENUMBER`
- **Screen Handling:** `WINDOW`

**Platforms:** Ohio Scientific Model 500, KIM-1, Atari 8-bit, Commodore 64, Commodore 128, and various 6502-based systems

---

### 11. BASIC-68 & BASIC-69 (1980s)

**Key Capabilities:**

- BASIC-68: Motorola 6800 processor version
- BASIC-69: Motorola 6809 processor version
- Features copied from BASIC-80
- Running FLEX operating systems

**Platforms:** Motorola 6800, Motorola 6809 systems, FLEX OS

---

### 12. Integer BASIC (1976-1979)

**Developer:** Steve Wozniak

**Version Range:** Single major version with minor updates

**Release Date:** 1976 (Apple I cassette), 1977 (Apple II ROM)

**Key Capabilities:**

- Integer-only arithmetic (16-bit signed, -32,767 to 32,767)
- **No floating-point support** (by design for speed)
- 2-3× faster than Applesoft BASIC
- Array-based strings (HP BASIC style)
- Low-resolution graphics: 40×40 color pixels
- Speaker control via memory peeking (`PEEK`/`POKE`)
- Unlimited-length variable names (unlike most BASIC variants)
- Hand-assembled in 6502 machine code for optimal performance
- Lacked error-trapping and user-defined functions

**Unique Characteristics:**

- Pure integer performance optimization
- Designed explicitly for games ("GAME BASIC")
- Fastest BASIC available for 6502 processors at the time
- Required transition to Applesoft for floating-point calculations

**Platforms:** Apple I (cassette storage), Apple II (ROM-based), Apple II Plus (disk-based executable)

**Legacy:** Replaced by Applesoft BASIC by 1979; largely historical but influential in microcomputer development

---

### 13. Applesoft BASIC (1977-1993+)

**Developer:** Marc McDonald and Ric Weiland (Microsoft), adapted by Randy Wigginton and Apple team

**Version Range:** 1.x through Applesoft II (1978 onward)

**Release Date:** 1977 (Version 1.0, ~8.5KB); Applesoft II (1978, ~10KB)

**Key Capabilities:**

- Floating-point mathematics (40-bit precision)
- Dynamic garbage-collected strings
- High-resolution graphics support (280×192 pixels)
- User-defined functions with `DEF FN` statement
- Error-trapping capability
- First 2 characters of variable names are significant (32-character variable names supported)
- Program save in tokenized binary format
- Control structures: `FOR...NEXT`, `GOSUB...RETURN`, `IF...THEN...ELSE`
- `INPUT` statement with optional prompts
- `DATA` and `READ` statements for data management
- `DIM` statement for array allocation
- Multiple file I/O (disk-based on later systems)

**Graphics Capabilities:**

- High-resolution mode: 280×192 monochrome
- Low-resolution mode: 40×40 color pixels
- `PLOT` and `HPLOT` commands for drawing
- `COLOR=` statement for color selection
- `FILL` command for filled shapes
- Animation support through page switching

**Sound and Input:**

- Speaker control via `PEEK`/`POKE` and timing loops
- Joystick input via `PADDLE` statement
- Keyboard input through `GET` and `INPUT` statements

**Special Features:**

- `CHAIN` command for loading and executing other programs
- `CALL` statement for machine language integration
- `SAVE` and `LOAD` commands for program persistence

**Unique Characteristics:**

- Microsoft BASIC ported to 6502 processor
- Slower than Integer BASIC but more versatile
- Standard BASIC for entire 16-year Apple II production run (1977-1993)
- Approximately 6 million units shipped with Applesoft BASIC

**Platforms:** Apple II series (all models 1977-1993), Apple II+, Apple IIe, Apple IIc, Apple IIGS; also Apple clones like Laser 128

---

### 14. Apple Business BASIC (1981)

**Developer:** Donn Denman (Apple)

**Version Range:** Single version for Apple III

**Release Date:** 1981 (exclusive to Apple III)

**Key Capabilities:**

**Numeric Types (unique to Business BASIC):**

- Floating-point (64-bit IEEE standard)
- String
- 16-bit signed integers
- 64-bit long integers

**Advanced Variable Support:**

- 64-character variable names (industry-leading for the era)
- Case-insensitive identifiers
- Type prefixes for explicit declaration

**File Handling:**

- Sequential file I/O
- Random-access files with record-based I/O
- Direct file and directory management without DOS commands
- `OPEN`, `CLOSE`, `READ`, `WRITE` statements
- File pointer control with `SEEK` statement

**Business-Oriented Features:**

- Currency support via long integers (×100 multiplier to avoid floating-point rounding errors)
- Advanced mathematical functions for financial calculations
- Modular programming via `CHAIN` for large applications
- External code integration: `INVOKE`/`PERFORM` for library functions
- Full-screen editing mode with Escape key navigation

**Editor and Environment:**

- Full-screen text editor
- Keystroke recording and playback for automation
- Program development without system command line
- Integrated debugging capabilities
- Direct Apple III SOS (Apple Sophisticated Operating System) integration

**Unique Characteristics:**

- First Apple BASIC explicitly designed for business and enterprise use
- **Complete incompatibility with Apple II** (different CPU: 6502 vs 8085 in Apple III)
- Sophisticated data handling for business applications
- No need for DOS commands for file management
- Most advanced BASIC variant of early 1980s for business applications

**Platforms:** Apple III only (not compatible with Apple II or Apple I)

**Legacy:** Limited commercial success due to Apple III market failure; remained largely unknown compared to Applesoft

---

## Professional Era (1985-1998)

### 15. QuickBASIC (1985-1990)

**Version Range:** 1.0 through 4.5, plus BASIC PDS 7.0 and 7.1

**Release Date:** August 18, 1985 (v1.0); Final: October 1990 (BASIC PDS 7.1)

**Key Capabilities:**

**Versions 2.0+:** Integrated Development Environment (IDE)

- On-screen text editor
- Direct editing in IDE
- Interpreter for testing

**Version 4.0:** Major enhancements

- Compiler included (PC BASIC Compiler)
- Interpreter for debugging before compilation

**Version 4.5 (1988):**

- Full IDE with debugging features
- PC BASIC Compiler for DOS executables
- Optional line numbers (line labels replaced line numbers)
- Control structures: multiline conditionals, loop blocks
- User-defined types
- Improved graphics and disk support
- Final standalone version

**BASIC PDS 7.x (Professional Development System):**

- Extended BASIC (QBX - QuickBASIC Extended)
- Versions up to 7.1
- Ran on DOS and OS/2

**QBasic (subset):**

- Version 4.5 interpreter-only version
- Included with MS-DOS 5.0+
- Limited to interpreter only
- Smaller program size limit
- No separate module support

**Macintosh Version (1988):**

- System 6 (min. 1 MB RAM) or System 7 (with 32-bit addressing disabled)
- Event-driven programming model for Macintosh

**Platforms:** MS-DOS (x86), Apple Macintosh (System 6, System 7)

---

### 16. QBasic (1991-1999)

**Version Range:** 1.0, 1.1

**Release Date:** 1991 (v1.0 with MS-DOS 5.0)

**Key Capabilities:**

- Interpreter-only (no compiler)
- Subset of QuickBASIC 4.5
- Structured programming support (SUB/END SUB)
- User-defined data types (limited)
- Modern IDE with debugger
- On-the-fly expression evaluation
- Code modification during debugging
- Backward compatible with DOS 3.20 and up
- Included example programs: Nibbles, Gorillas, MONEY MANAGER, RemLine

**Platforms:** MS-DOS, PC DOS, OS/2, Windows 95, Windows 98, Windows Me, Windows NT 3.x, Windows NT 4.0

**Availability:**

- v1.0: MS-DOS 5.0+, Windows 95, NT 3.x, NT 4.0
- v1.1: MS-DOS 6.x, Windows 95/98/Me
- Discontinued with Windows 2000

---

### 17. Visual Basic Classic (1991-2008)

**Version Range:** 1.0 through 6.0

**Release Date:** May 1991 (Windows v1.0); Final: Mid-1998 (v6.0)

**Version Progression:**

**Visual Basic 1.0 (May 1991) - Windows:**

- First Windows version
- Drag-and-drop GUI design
- Event-driven programming
- Text-based user interface

**Visual Basic 1.0 (September 1992) - DOS:**

- Text-based user interface
- Next version of QuickBASIC/BASIC PDS
- Not fully compatible with Windows version

**Visual Basic 2.0 (November 1992):**

- Improved IDE usability
- Forms became instantiable objects
- Foundational class module concepts

**Visual Basic 3.0 (Summer 1993):**

- Standard and Professional versions
- Jet Database Engine 1.1 (Access 1.x compatibility)
- Synchronized with Windows 3.1

**Visual Basic 4.0 (August 1995):**

- Standard, Professional, Enterprise editions
- First 32-bit and 16-bit support
- Separated language from GUI (VBA)
- Non-GUI class support
- Embedded in Office 95 suite
- VBX → OLE/ActiveX controls (`.OCX` files)

**Visual Basic 5.0 (February 1997):**

- 32-bit only (could import VB4 16-bit code)
- Custom user controls support
- Native code compilation option
- Control Creation Edition (free)
- P-Code compilation

**Visual Basic 6.0 (Mid-1998):**

- Enhanced web application capabilities
- Native code compilation improved
- All versions used VB runtime (MSVBVM50.DLL or MSVBVM60.DLL)

**Core Features (All Versions):**

- Rapid Application Development (RAD)
- Visual form design
- Event handlers
- Database access (DAO, RDO, ADO in later versions)
- COM component support
- ActiveX control creation
- Windows API access
- Case-insensitive code
- Named code blocks (Sub...End Sub, Function...End Function)
- No line numbers required
- Multi-line statements with `_`
- Comment syntax: `'`
- Control structures: Do...Loop, While...End While, For...Next, If...Then...Else...End If, Select Case

**Platforms:** MS-DOS, Microsoft Windows, Apple Macintosh (v1.0)

**Support Status:**

- Mainstream support ended: March 31, 2005
- Extended support ended: March 2008
- VB6 runtime supported through Windows 11

---

## Modern Era (2002+)

### 18. Visual Basic .NET (2002-Present)

**Version Range:** 7.0 through 16.x and beyond

**Version History and Features:**

| Version | Release Year | Key Features |
|---------|--------------|--------------|
| 7.0 | 2002 | First .NET version, managed code, .NET Framework 1.0 |
| 7.1 | 2003 | .NET Framework 1.1, Compact Framework support, improved IDE |
| 8.0 | 2005 | Edit and Continue, `My` namespace, Generics, Partial classes, Operator overloading, Nullable types, `IsNot` operator |
| 9.0 | Nov 2007 | .NET Framework 3.5, Conditional operator, Anonymous types, LINQ support, Lambda expressions, XML literals, Type inference, Extension methods |
| 10.0 | April 2010 | Dynamic Language Runtime interop, Inline functions/subs, Multi-line lambdas, Line continuation inference |
| 11.0 | 2012 | .NET Framework 4.5, Async/await, Iterators, Call hierarchy, Caller information |
| 12.0 | 2013 | .NET Framework 4.5.1 |
| 14.0 | 2015 | Null-conditional operator `?.`, String interpolation, .NET Framework 4.6 |
| 15.x | 2017 | Refactoring tools, Enhanced language features |
| 16.0 | 2019 | .NET Core 3.0 focus, Init-only properties (v16.9) |
| 17+ | 2022+ | Maintenance mode, continued .NET support |

**Core Capabilities:**

- Multi-paradigm: structured, imperative, OOP, declarative, generic, reflective, event-driven
- Managed code on .NET Framework/Core
- Windows Forms and web application development
- Console applications
- Object-oriented (classes, inheritance, polymorphism)
- LINQ support
- Async/await patterns
- Strong typing with optional weak typing
- Drag-and-drop form designer

**Platforms:** .NET Framework, Mono, .NET Core, and later .NET versions (Windows, Linux, macOS, Android, iOS, BSD, Solaris, Unix)

**IDE:** Visual Studio (various editions, free Community edition available)

---

### 19. Small Basic (2008-Present)

**Version Range:** 1.0 and later

**Release Date:** October 2008

**Key Capabilities:**

- Simplified BASIC for education
- Only 14 keywords
- Designed for ages 8+
- Graphics support
- Turtle graphics
- Event handling
- Educational focus
- Gateway to Visual Basic and Visual C#

**Platforms:** Windows

---

## Summary Timeline

| Name | Version | Year | Type | Platform |
|------|---------|------|------|----------|
| Altair BASIC | 2.0-5.0 | 1975 | Interpreter | Altair 8800, S-100 |
| Integer BASIC | Single | 1976 | Interpreter | Apple I, Apple II |
| Applesoft BASIC | 1.x, II | 1977 | Interpreter | Apple II series |
| IBM Cassette BASIC | C1.00-C1.20 | 1981 | Interpreter | IBM PC, XT, PS/2 |
| IBM Disk BASIC | 1.0-3.30 | 1981 | Interpreter | IBM PC DOS |
| IBM BASICA | 1.0-3.30 | 1981 | Interpreter | IBM PC DOS |
| Apple Business BASIC | Single | 1981 | Interpreter | Apple III |
| IBM PCjr Cartridge BASIC | — | 1984 | Interpreter | IBM PCjr |
| GW-BASIC | 1.0-3.23 | 1983 | Interpreter | MS-DOS (PC compatibles) |
| MBASIC | 5.x+ | 1976+ | Interpreter/Compiler | CP/M |
| BASIC-86 | — | 1979 | Standalone | 8086 systems |
| TRS-80 BASIC | Level II-III | 1977+ | Interpreter | Tandy TRS-80 |
| BASIC-68/69 | — | 1980s | Interpreter | Motorola 6800/6809 |
| 6502 BASIC | 8K-9K variants | 1976 | Interpreter | 6502-based systems |
| QuickBASIC | 1.0-4.5, PDS 7.x | 1985-1990 | Compiler/Interpreter | MS-DOS, Macintosh |
| QBasic | 1.0-1.1 | 1991 | Interpreter | MS-DOS, Windows 95+, OS/2 |
| Visual Basic Classic | 1.0-6.0 | 1991-1998 | Compiler | Windows, DOS, Macintosh |
| Visual Basic .NET | 7.0-16.x+ | 2002+ | Compiler | .NET Framework/Core, Cross-platform |
| Small Basic | 1.0+ | 2008+ | Interpreter | Windows |

---

## Key Evolution Highlights

### From Single-Language to Ecosystems

- **1975-1983:** Hardware-specific implementations for various processors and systems
- **1983-1990:** Standardization around IBM PC and DOS; QuickBASIC introduced modern IDE concepts
- **1991-1998:** Visual Basic revolutionized BASIC with visual form design and event-driven programming
- **2002+:** Visual Basic .NET moved to managed runtime; Small Basic introduced simplified education-focused variant

### Major Capability Expansions

- **Graphics:** From simple pixel operations → CGA/EGA support → Full GUI designers
- **Sound:** From basic frequency tones → Music macro language (PLAY statement) → Modern audio APIs
- **Development:** From line-based editors → Full IDEs with debugging → Visual designers with IntelliSense
- **Type System:** From simple variables → User-defined types → Full OOP with classes and inheritance

### Platform Evolution

- Started on hardware-specific systems (Altair, TRS-80, Commodore)
- Consolidated around IBM PC/DOS
- Moved to Windows platform
- Recently diversified to cross-platform .NET Core

---

## Sources

- **Wikipedia Articles:**
  - [Microsoft BASIC](https://en.wikipedia.org/wiki/Microsoft_BASIC)
  - [Altair BASIC](https://en.wikipedia.org/wiki/Altair_BASIC)
  - [IBM BASIC](https://en.wikipedia.org/wiki/IBM_BASIC)
  - [GW-BASIC](https://en.wikipedia.org/wiki/GW-BASIC)
  - [QuickBASIC](https://en.wikipedia.org/wiki/QuickBASIC)
  - [QBasic](https://en.wikipedia.org/wiki/QBasic)
  - [Visual Basic (classic)](https://en.wikipedia.org/wiki/Visual_Basic_(classic))
  - [Visual Basic (.NET)](https://en.wikipedia.org/wiki/Visual_Basic_(.NET))
  - [Integer BASIC](https://en.wikipedia.org/wiki/Integer_BASIC)
  - [Applesoft BASIC](https://en.wikipedia.org/wiki/Applesoft_BASIC)
  - [Apple III](https://en.wikipedia.org/wiki/Apple_III) (for Business BASIC)

- **Other Sources:**
  - WinWorld PC (Software Archive)
  - Microsoft official documentation
  - BYTE Magazine archives
  - Microsoft Press publications

---

## Apple Computer BASIC Variants

### 1. Integer BASIC

**Developer:** Steve Wozniak

**Release Date:** 1976 (Apple I), 1977 (Apple II)

**Platform(s):**

- Apple I (cassette, 1976)
- Apple II (ROM, 1977)
- Apple II Plus (available as executable on DOS 3.3 disk after ROM removal in 1979)

**Version(s):**

- Single version, no numbered releases, with minor ROM updates for different hardware

**Key Capabilities:**

- **Numeric System:** Integer-only arithmetic
  - 16-bit signed integers: -32,767 to 32,767
  - NO floating-point support
  - Extremely fast performance vs. contemporary BASICs
  - Compact 16-bit storage format

- **String Handling:** Array-based (HP BASIC style)
  - Strings as character arrays requiring `DIM` declaration
  - Fixed-size allocation (up to 255 characters)
  - Array slicing syntax for substring access: `A$(0,5)` for first 6 characters
  - No automatic garbage collection needed (memory-efficient)
  - Lacked `CHR$` function (missing ASCII-to-character conversion)

- **Variables:** Unusual for the era
  - Variable names of any length (e.g., `GAMEPOINTS`, `PLAYER2`)
  - Constraint: names couldn't contain reserved words as substrings

- **Graphics Capabilities:**
  - Low-resolution graphics mode: 40×40 color pixel grid
  - Commands: `GR` (graphics mode), `TEXT` (text mode), `COLOR=` (set color 0-15)
  - `PLOT x,y` - single pixel
  - `HLIN x1,x2 AT y` - horizontal line
  - `VLIN y1,y2 AT x` - vertical line
  - `SCRN(x,y)` - read pixel color
  - Mixed text (4 lines bottom) and graphics display support
  - Window control via `POKE` to memory locations 32-35

- **Sound Capabilities:**
  - No dedicated sound hardware (only speaker beeper)
  - `PEEK(-16336)` memory manipulation for speaker control
  - Clicking speaker repeatedly produced baritone tones
  - Limited to simple beeps and buzzes via software toggling

- **Program Control:**
  - Commands: `RUN`, `CON` (continue), `TRACE`/`NOTRACE`, `DSP`/`NODSP`, `POP`
  - `AUTO` for automatic line numbering with `MAN` to disable
  - Line numbers: 0-32,767
  - Line length: max 128 characters

- **Input/Output:**
  - Limited I/O; no `DATA`/`READ` statements
  - `PR#x` and `IN#x` for peripheral slot redirection
  - `PDL(n)` - read paddle controller (0 or 1, returns 0-255)

- **Special Features:**
  - Included machine code monitor, mini-assembler, and disassembler in ROM
  - Floating-point library available in ROM (users could `CALL` into it)
  - `TAB n` command (not function) for column positioning (0-39)
  - `VTAB n` for vertical positioning (1-24)
  - Hand-assembled by Wozniak (no assembler initially available)

- **Performance:** Superior to contemporary BASICs
  - 2-3× faster than Applesoft BASIC on same hardware
  - Integer math faster than floating-point operations
  - More complete tokenization reduced runtime parsing

- **Intended Use:** Games and educational programs (explicitly called "GAME BASIC" during development)

**Storage Media:**

- Cassette tape (Apple I)
- ROM (Apple II onward)
- Disk (after 1979, as executable file on DOS 3.3)

**Distinguishing Characteristics:**

- **Only BASIC with pure integer arithmetic** on a 6502-based machine at the time
- Fastest performing BASIC relative to other implementations
- Required manual programming to call floating-point routines for scientific applications
- Unique array-based string model (predated C and Fortran, more similar to HP BASIC)
- Exceptionally fine-grained tokenization improved performance but added complexity

**Known Limitations:**

- No floating-point math (critical limitation)
- Limited string handling (fixed-size, no dynamic allocation)
- Sparse math functions (only `ABS`, `SGN`, `RND`)
- No error-trapping mechanism
- No user-defined functions
- Small standard library

**Comparison to Applesoft BASIC:**

| Feature | Integer BASIC | Applesoft BASIC |
|---------|---------------|-----------------|
| Numeric Types | Integer only | Floating-point (40-bit), Integer with `%` |
| String Model | Fixed-size arrays | Dynamic garbage-collected |
| Floating-Point | Via CALL to ROM routines | Native support |
| Performance | 2-3× faster | Slower due to floating-point overhead |
| Arrays | Single-dimension only | Multi-dimensional |
| User Functions | None | `DEF FN` (single-line) |
| Error Trapping | None | `ONERR...GOTO` and `RESUME` |
| Graphics Commands | Low-res only | High-resolution graphics support |
| Variable Names | Unlimited length | Effective 2-character limit (first 2 significant) |
| Intended Use | Games, education | General-purpose, business applications |
| Compatibility | No MS BASIC compatibility | Backwards-compatible with Integer BASIC intent |

**Historical Context:**

- Replaced by Applesoft BASIC starting with Apple II Plus (1979)
- Phased out to make ROM space for floating-point BASIC
- Beloved by game developers for its speed
- Limited applicability for scientific/business computing due to integer-only arithmetic

---

### 2. Applesoft BASIC (FP BASIC)

**Developer(s):** Marc McDonald, Ric Weiland (Microsoft) + Randy Wigginton and Apple team (adaptation)

**Release Date:** 1977 (original Applesoft), 1978 (Applesoft II)

**Platform(s):**

- Apple II (1977 on cassette)
- Apple II Plus (1979 in ROM)
- Apple IIe (1983)
- Apple IIc (1984)
- Apple IIGS (1986)
- Apple II clones (Laser 128, VTech systems)

**Version(s):**

1. **Applesoft (v1.x) - 1977 (8.5 KB)**
   - Original, released on cassette
   - Smaller interpreter footprint
   - Different low-res graphics command naming (PLTx format: PLTG, PLTC, PLTP, PLTH, PLTV)
   - Lacked high-resolution graphics support (9 commands missing)
   - No error-trapping (`ONERR`/`RESUME`)
   - No machine shorthand (`&` command)
   - No `HOME` command
   - Text controls: `NORMAL`, `INVERSE`, `FLASH`, `SPEED=`
   - Bitwise operators (`AND`, `OR`, `NOT`) operated on 16-bit values only
   - `USR()` function different from later versions (no address pre-definition)
   - Memory Size? prompt and Microsoft copyright notice displayed

2. **Applesoft II - 1978 (10 KB, standard)**
   - Enhanced version, cassette and disk
   - Became ROM standard with Apple II Plus (1979)
   - Added all high-resolution graphics commands (9 commands)
   - Added `ONERR`/`GOTO`/`RESUME` error-trapping
   - Added `&` command for machine code shorthand
   - Added `HOME` command
   - Improved command set
   - Compatible with Integer BASIC programs (with caveats)
   - This version became synonymous with "Applesoft BASIC"

**Key Capabilities:**

- **Numeric System:** Floating-point primary
  - 40-bit single-precision floating-point (8-bit exponent, 31-bit significand)
  - Trigonometric functions: `SIN`, `COS`, `ATN`, `TAN`
  - Logarithmic functions: `LOG`, `EXP`, `SQR`
  - Integer variables with `%` suffix (2-byte, 16-bit)
  - Internal conversion between types (performance penalty)
  - Slower than Integer BASIC due to floating-point overhead

- **String Handling:** Modern garbage-collected approach
  - Atomic string type (not array-based)
  - Variable-length strings
  - `$` suffix for string variables
  - Multi-dimensional string arrays: `DIM A$(10)`
  - Functions: `CHR$`, `STR$`, `VAL`, `ASC`, `LEFT$`, `RIGHT$`, `MID$`, `LEN`
  - Concatenation with `+` operator
  - Automatic garbage collection (notoriously slow in some versions, "completely broken" in others)

- **Graphics Capabilities:** Extensive support
  - **Low-resolution mode:** 40×40 pixel grid, 16 colors
    - `GR` - enter graphics mode
    - `TEXT` - return to text
    - `COLOR=n` - set color (0-15)
    - `PLOT x,y` - draw pixel
    - `HLIN x1,x2 AT y` - horizontal line
    - `VLIN y1,y2 AT x` - vertical line

  - **High-resolution mode (Applesoft II only):** 280×192 resolution, 2 colors per scan line
    - `HRES` - high-resolution mode
    - `HPLOT x,y` - plot in high-res
    - `HLIN` and `VLIN` for high-res lines
    - Shape tables for scaled and rotated objects
    - Arbitrary line drawing capability
    - Limited text overlay (4 lines at bottom only)

  - **Double-high resolution mode (Apple IIe with 128KB):** Duplicates high-res with all 16 colors
    - No direct Applesoft support (requires machine code)

  - Mixing text and graphics: 4 lines of text at bottom of graphics screen

- **Sound Capabilities:**
  - No dedicated sound hardware
  - ASCII bell character (`PRINT CHR$(7)`) produces system beep
  - `PEEK(-16336)` for speaker clicking (baritone buzzes only)
  - Programs could embed machine-language routines for multi-voice music
  - Fast enough to produce low-frequency electronic tones via software

- **Program Control:**
  - `RUN`, `GOTO`, `GOSUB`/`RETURN`, `FOR`/`NEXT`, `IF`/`THEN`
  - `TRACE`/`NOTRACE` for debugging
  - Multiple statements per line (separated by `:`)
  - `?` as shortcut for `PRINT` (tokenized to same code)

- **Data & Variables:**
  - `DATA` statements with `READ` and `RESTORE` commands
  - Single and multi-dimensional arrays
  - Variable names: **first 2 characters significant only**
  - Limitation: "LOW" and "LOSS" treated as identical
  - Reserved word collision issue: "SCORE" parsed as "SC OR E"
  - No lowercase letters in programs (except strings) in early models

- **Input/Output:**
  - Standard `INPUT`, `PRINT` commands
  - `PR#x` output redirection to slot x (printer, disk, 80-column card)
  - `IN#x` input redirection from slot x
  - `PR#0` and `IN#0` restore to screen/keyboard
  - Cassette save/load (minimal, no DOS integration)
  - Disk I/O via DOS commands (not BASIC statements)

- **Advanced Features:**
  - `ONERR...GOTO` and `RESUME` for error-trapping (with known bugs)
  - User-defined functions: `DEF FN` (single-line only)
  - `CALL` for machine language subroutine access
  - `USR()` function for calculated return values from machine code
  - `&` shorthand for `CALL` with pre-defined address
  - Ampersand routines via third-party extensions
  - `PEEK` and `POKE` for memory access

**Storage Media:**

- Cassette tape (1977 original)
- 5.25" floppy disk (via Disk II, 1978+)
- ROM (Apple II Plus 1979+)

**Distinguishing Characteristics:**

- **Microsoft BASIC port:** 6502 version of Altair BASIC
- **Floating-point-first architecture:** Unlike Integer BASIC
- **Backwards-compatible intent:** Designed to run Integer BASIC programs
- **"FP BASIC":** Called this in DOS context to distinguish from Integer BASIC (`INT` command)
- **Rainbow logo popularity:** Standard BASIC for most Apple II owners
- **Performance trade-off:** Slower than Integer BASIC, but more versatile

**Known Limitations:**

- 2-character variable name limit (practical limitation despite syntax allowing more)
- Lowercase support missing in early versions (only uppercase)
- Missing `INKEY$` (check keystroke without blocking)
- No `PRINT USING` (formatted output)
- No `INSTR` (substring search, added later)
- No `LPRINT` (printer output direct)
- No bitwise operators
- Predictable `RND` generator (seeding issue documented)
- `ONERR` bug: system stack not reset if `RESUME` not called (crash risk)
- Garbage collection slowness (notorious performance issue)
- Linear `GOTO`/`GOSUB` lookup (slow subroutine calls in large programs)
- ASCII literal numbers not converted to binary on entry (parsed at runtime)

**Comparison to Integer BASIC:**

| Aspect | Integer BASIC | Applesoft BASIC |
|--------|---------------|-----------------|
| Primary Use | Games | General-purpose |
| Floating-Point | No (call ROM) | Yes (native) |
| Graphics | Low-res only | High-res support |
| String Type | Fixed arrays | Dynamic, atomic |
| Speed | 2-3× faster | Slower |
| Math Functions | Sparse | Comprehensive |
| Variable Names | Unlimited | 2-char significant |
| User Functions | No | Yes (DEF FN) |
| Intended Replacement | None | Yes, phased Integer BASIC |

**Historical Evolution:**

- Licensed from Microsoft for $31,000 (8-year flat fee, 1977)
- Renewal in 1985 gave Microsoft Macintosh BASIC source code
- Original v1.x (1977): smaller, simpler, lacked graphics
- v2.x/II (1978): expanded, full graphics, standard version
- Became dominant BASIC for Apple II ecosystem
- Compilers available: TASC (The Applesoft Compiler, Microsoft 1981)
- Chinese-localized version: Chinese BASIC
- Compatible clones: Coleco Adam SmartBASIC, VTech Laser 128

---

### 3. Apple Business BASIC

**Developer:** Donn Denman

**Release Date:** 1981

**Platform(s):**

- Apple III (primary platform, built-in)
- Apple III under CP/M (expanded Microsoft BASIC also available)

**Version(s):**

- Version 1.0 (1981, only major release documented)
- No sub-versions or updates documented in historical records

**Key Capabilities:**

- **Purpose & Use Case:** Enterprise business and productivity software
  - Heavy-duty program development (hundreds of lines of code)
  - Business applications with complex data handling
  - File and database operations without DOS commands
  - Large program support via modular design

- **Numeric System:** Four distinct types (expanded vs. Applesoft)
  - **Floating-point (real):** 40-bit standard
  - **Integer (short):** 16-bit, denoted by `%` suffix
  - **Long Integer:** 64-bit, denoted by `&` suffix (19-decimal-digit precision)
  - **Currency support:** Long integers used to avoid rounding errors
    - Values stored × 100 (0.01 = 1, 10.10 = 1010)
    - `PRINT USING` and `IMAGE` statements format decimal output without conversion

- **String Handling:** Enhanced vs. Applesoft
  - Atomic strings (like Applesoft)
  - Garbage-collected variable-length
  - No 2-character limit on variable names

- **Variable Names:** Industry-leading for the era
  - Up to 64 characters, all significant
  - Case-insensitive (`A.Variable` = `a.VARIABLE`)
  - Allowed punctuation (periods for clarity): `This.Is.A.Variable.Name`
  - Fully qualified name support in very long programs

- **Data Types & Conversions:**
  - Typed variables (can hold only one type)
  - Automatic conversions where possible (floating ↔ integer)
  - `CONV` functions for explicit type conversion:
    - `CONV$()` - convert to string
    - `CONV%()` - convert to integer
    - `CONV&()` - convert to long integer
    - `CONV()` - convert to floating-point

- **Graphics Capabilities:** Limited (inherited from Applesoft)
  - Low-resolution: `GR`, `COLOR=`, `PLOT`, `HLIN`, `VLIN`
  - Text/graphics mixing (4 lines bottom)
  - No high-resolution support mentioned

- **Sound Capabilities:** Limited
  - System beep via ASCII bell character
  - Speaker clicking via `PEEK(-16336)`
  - No dedicated sound support

- **File Handling:** Advanced, primary differentiator from Applesoft
  - **Sequential file access:**
    - `OPEN# n AS [INPUT|OUTPUT], filename`
    - `CLOSE#` (single file) or `CLOSE` (all files)
    - `INPUT#` / `READ#` for input
    - `PRINT#` / `WRITE#` for output

  - **Random-access (fixed-width record) files:**
    - `CREATE filename, type, record_length`
    - Record length specified at creation (e.g., 500 bytes)
    - All reads/writes padded to record length
    - `INPUT#n,record#;variable` for numbered record access
    - System variables: `TYP` (next var type), `REC` (last record accessed)

  - **File management (no DOS exit required):**
    - `CATALOG path` - directory listing
    - `DELETE filename` - remove file
    - `RENAME oldname,newname`
    - `LOCK`/`UNLOCK` - file protection

  - **Directory management:**
    - `CREATE` could create directories too
    - `PREFIX$` variable for path pre-pending to all filenames

  - **End-of-File handling:**
    - `ON EOF# n ... OFF EOF#` - specialized trap for EOF errors
    - `EOF` variable contains file number causing error

  - **Error handling:**
    - `ON ERR ... END ERR` - general error trap (from Applesoft)
    - File-specific: `ON EOF#` specialized trap

- **Program Editing:**
  - Command prompt: `)` (vs. `>` for Integer, `]` for Applesoft)
  - Full-screen editing mode: Escape key + cursor keys
  - `INDENT` variable - leading spaces in `FOR...NEXT` blocks
  - `OUTREC` variable - max line width on `LIST` output
  - `DEL range` - mass line deletion (e.g., `DEL 100 TO 500`)
  - `TRACE`/`NOTRACE` - execution tracking (displays `#linenum` as lines execute)

- **Debugging Support:**
  - `TRACE` - prints `#` + line number during execution
  - `NOTRACE` - disable tracing
  - Allows watching variable changes via `PRINT` with `TRACE`

- **Program Extension (Modular Code):**
  - `CHAIN filename [, line_number]` - load and execute program, preserves variables
    - Optional starting line (default: from program start)
    - Allows segmented programming for large applications
    - Risk: global variable namespace conflicts (no `COMMON` protection in Business BASIC)

  - `EXEC filename` - load text file, parse commands (`LOAD`, `RUN`, or code lines)
    - Enables program merging (slower than `CHAIN`)
    - Allows dynamic code generation

  - `OUTPUT # handle` - redirect all output to file
    - Can capture `LIST` to external file for backups/version control

- **External Code Integration:**
  - `INVOKE filename` - load machine language code into memory
    - Multiple files: comma-separated list in single statement
    - Supports external libraries/routines

  - `PERFORM routine_name(args)` - call loaded routine
    - Arguments in parentheses like function call
    - Integer/long arguments prefixed: `%variable`, `&variable`
    - Floating-point/string arguments not prefixed

  - `EXFN` - call routine returning floating-point
  - `EXFN%` - call routine returning integer

- **Operators & Functions:**
  - **Arithmetic:** `+`, `-`, `*`, `/`, `^` (exponent)
  - **Logical:** `AND`, `OR`, `NOT`
  - **Comparison:** `=`, `>`, `<`, `>=`, `<=`, `<>` (also alternate forms: `=>`, `=<`, `><`)
  - **Special arithmetic:** `MOD` (remainder), `DIV` (integer division)
  - **Math functions:** `SIN`, `COS`, `TAN`, `ATN`, `RND`, `INT`, `ABS`, `SQR`, `EXP`, `LOG`
  - **Conversion:** `HEX$()` (to hex), `TEN()` (from hex)
  - **String functions:** `LEFT$`, `RIGHT$`, `MID$`, `LEN`, `ASC`, `CHR$`, `VAL`, `STR$`, `INSTR` (search), `SUB$` (overwrite)

- **User-Defined Functions:**
  - `DEF FN` syntax supported

- **Reserved Variables (System Variables):**
  - `INDENT` - leading spaces in loops
  - `OUTREC` - output line width
  - `HPOS`, `VPOS` - cursor horizontal/vertical position (settable)
  - `PREFIX$` - path pre-pend for all file operations
  - `EOF` - file number of EOF error
  - `TYP` - next variable type in random-access file
  - `REC` - last record number read/written

**Program Line Management:**

- Line numbers: 0-63,999 (higher range than Applesoft's 0-32,767)
- Full-screen editing with cursor keys (Escape to enter edit mode)
- Automatic indentation support for readability

**Storage Media:**

- Floppy disk (5.25" via Apple III hardware)
- Native disk commands (not DOS-dependent)

**Distinguishing Characteristics:**

- **Business-focused:** Only Apple BASIC designed explicitly for enterprise use
- **64-character variable names:** Industry-leading feature for the era
- **Long integer support:** Unique feature addressing currency/precision needs
- **Sophisticated file handling:** Random-access, record-based files without DOS
- **Modular program architecture:** `CHAIN` for large application development
- **External code integration:** `INVOKE`/`PERFORM` for libraries
- **Advanced debugging:** `TRACE` without line-by-line stepping

**Comparison to Applesoft BASIC:**

| Feature | Applesoft | Apple Business |
|---------|-----------|----------------|
| Variable Name Length | 2 chars significant | 64 chars, all significant |
| Data Types | 2 (float, string) | 4 (float, string, int, long) |
| Integer Support | Via `%` suffix only | Full `%` and `&` (long) types |
| Currency Support | No | Yes (long int × 100) |
| File I/O | Cassette only | Sequential + random-access |
| File Management | Via DOS | Direct `CREATE`, `DELETE`, `RENAME` |
| Line Number Range | 0-32,767 | 0-63,999 |
| Full-Screen Editing | No | Yes (Escape key) |
| Program Tracing | No | Yes (`TRACE`/`NOTRACE`) |
| `CHAIN` (modular programs) | No | Yes |
| External Code Loading | Via `CALL` only | `INVOKE`/`PERFORM` library model |
| Target Use | General home computing | Business/enterprise applications |
| Graphics | Full (low & high-res) | Limited (low-res) |
| High-Res Graphics | Yes | No |

**Intended Use Case:**

- Multi-user business applications
- Database and file processing programs
- Point-of-sale systems
- Inventory management
- Payroll and accounting systems
- Programs exceeding typical home computer scope

**Known Limitations:**

- No `COMMON` for `CHAIN` - variable namespace conflicts possible
- Modest graphics capabilities vs. Applesoft
- Tightly coupled to Apple III (not available on Apple II)
- Limited external documentation/community (business focus)
- Random-access file implementation complex vs. contemporary databases
- No structured programming constructs (no procedures/modules beyond `CHAIN`)

**Historical Context:**

- Released as primary BASIC for Apple III computer (1981)
- Designed by Donn Denman at Apple
- Represented evolution of business BASIC genre (Data General, Wang, others)
- Apple III also offered expanded Microsoft BASIC under CP/M (via 3rd party support)
- Less well-known than Applesoft due to Apple III's limited market success
- Never ported to Apple II or other platforms

---

## Summary: Apple BASIC Variants

**Timeline:**

- **1976:** Integer BASIC (Apple I)
- **1977:** Integer BASIC (Apple II ROM), Applesoft BASIC (cassette)
- **1978:** Applesoft II (cassette/disk)
- **1979:** Applesoft II (Apple II Plus ROM, Integer BASIC replaced)
- **1981:** Apple Business BASIC (Apple III)

**Architecture Progression:**

1. **Integer BASIC:** Performance-optimized, integer-only, fast, game-focused
2. **Applesoft:** Floating-point versatility, slower, general-purpose, Microsoft-based
3. **Apple Business BASIC:** Enterprise features, modular, file-intensive, data-focused

**Hardware Support Summary:**

| BASIC | Apple I | Apple II | Apple II+ | Apple IIe | Apple IIc | Apple IIGS | Apple III |
|-------|---------|----------|-----------|-----------|-----------|------------|-----------|
| Integer | YES (ROM) | YES (ROM) | EXEC | - | - | - | - |
| Applesoft | - | YES (cassette) | YES (ROM) | YES (ROM) | YES (ROM) | YES (ROM) | - |
| Business | - | - | - | - | - | - | YES (built-in) |

---

**Document Compiled:** December 17, 2025

**Total Versions Documented:** 19 major versions spanning 50 years of BASIC history

- **Microsoft BASIC Variants:** 16 versions (Altair through Small Basic)
- **Apple BASIC Variants:** 3 versions (Integer BASIC, Applesoft, Apple Business BASIC)
- **Coverage Period:** 1975-2025

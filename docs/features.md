# MSBasic Implementation Status

This document tracks the implementation status of Applesoft BASIC compatibility in MSBasic.

## Core Architecture

- [x] Tokenizer → Parser → Interpreter pipeline
- [x] Shared AST for interactive and program execution
- [x] Custom 40-bit floating point (`src/float40.*`)
- [x] Number/string `Value` type system
- [x] Variable name normalization (2 chars significant, case-insensitive)
- [x] String variables (`$` suffix)
- [x] Integer variables (`%` suffix, 16-bit signed range)
- [x] Multiple statements per line (colon `:` separator)

## Expression Evaluation

- [x] Arithmetic operators: `+`, `-`, `*`, `/`, `^`
- [x] String concatenation: `+` operator for strings
- [x] MOD operator
- [x] Unary operators: `+`, `-`, `NOT`
- [x] Comparison operators: `=`, `<>`, `<`, `>`, `<=`, `>=`
- [x] Logical operators: `AND`, `OR`

## Standard Applesoft Commands

### Control Flow
- [x] GOTO line_number
- [x] GOSUB line_number
- [x] RETURN
- [x] ON expression GOTO/GOSUB line_number[,line_number...]
- [x] IF condition THEN statement/line_number
- [x] IF...THEN...ELSE
- [x] FOR variable = start TO end [STEP increment]
- [x] NEXT [variable]
- [x] WHILE condition
- [x] WEND
- [x] END
- [x] STOP
- [x] CONT
- [x] POP

### Variable and Data Management
- [x] LET variable = expression (LET is optional)
- [x] DIM array(size[,size...])
- [x] Arrays auto-dimension to size 10 if undeclared
- [x] DATA value[,value...]
- [x] READ variable[,variable...]
- [x] RESTORE [line_number]
- [x] CLR (same as CLEAR)
- [x] CLEAR

### User-Defined Functions
- [x] DEF FN name(parameter) = expression
- [x] FN name(argument) - function calls

### Input/Output
- [x] PRINT [expression[;|,expression]...]
- [x] ? (shorthand for PRINT)
- [x] INPUT ["prompt";] variable[,variable...]
- [x] TAB(n) - output spacing
- [x] SPC(n) - output spacing
- [x] Semicolon separator (tight spacing)
- [x] Comma separator (tab positions)

### Program Management
- [x] NEW - clear program
- [x] RUN [line_number]
- [x] LIST [start[,end]]
- [x] DEL start,end - delete line range
- [x] REM comment

### File Operations
- [x] LOAD filename
- [x] SAVE filename
- [x] CATALOG
- [x] RECALL arrayname - load array from file
- [x] STORE arrayname - save array to file

### Screen Control
- [x] HOME - clear screen and home cursor
- [x] HTAB column - horizontal tab
- [x] VTAB row - vertical tab
- [x] INVERSE - inverse video mode
- [x] NORMAL - normal video mode
- [x] FLASH - flashing text mode
- [x] TEXT - switch to text mode

### Graphics - Low Resolution
- [x] GR - 40x40 low-res graphics mode
- [x] COLOR= expression - set plotting color (0-15)
- [x] PLOT x,y - plot a point
- [x] HLIN x1,x2 AT y - horizontal line
- [x] VLIN y1,y2 AT x - vertical line

### Graphics - High Resolution
- [x] HIRES - enable high-resolution graphics mode
- [x] HGR - 280x192 hi-res graphics (mixed mode)
- [x] HGR2 - 280x192 hi-res graphics (full screen)
- [x] HCOLOR= expression - set hi-res color
- [x] HPLOT x,y [TO x,y]... - plot points/lines
- [x] MOVE x,y - move graphics cursor
- [x] ROTATE angle - set rotation for shapes (0-63)
- [x] SCALE size - set scale for shapes (1-255)
- [x] DRAW shape_number [AT x,y] - draw shape
- [x] XDRAW shape_number [AT x,y] - XOR draw shape
- [x] SHLOAD - load shapes from DATA statements
- [x] Shape table binary file loading

### Low-Level System Access
- [x] PEEK(address) - read memory byte
- [x] POKE address,value - write memory byte
- [x] CALL address - execute machine code
- [x] GET variable - read single keystroke
- [x] WAIT address,mask[,timeoutMs] - wait for memory condition

### Memory Management
- [x] HIMEM: address - set high memory limit
- [x] LOMEM: address - set low memory limit
- [x] Memory bounds enforcement for PEEK/POKE/WAIT

### Error Handling
- [x] ONERR GOTO line_number
- [x] RESUME - continue after error
- [x] Error code storage in memory location 222
- [x] Error line storage in locations 218-219

### Debugging and Timing
- [x] TRACE - show line numbers during execution
- [x] NOTRACE - turn off tracing
- [x] SPEED n - set execution speed delay (0-255 ms)
- [x] RANDOMIZE seed - initialize RND

### Device Control
- [x] PR# slot - set output device (stub)
- [x] IN# slot - set input device (stub)

## ProDOS Commands

### File System Operations
- [x] - (DASH) pn [,S#] [,D#] - run program without clearing variables
- [x] APPEND pn - prepare file for appending
- [x] BLOAD pn [,A#] [,S#] [,D#] - load binary file
- [x] BRUN pn [,A#] [,S#] [,D#] - load and run binary file
- [x] BSAVE pn,A#,L# [,S#] [,D#] - save binary file
- [x] CAT [pn] [,S#] [,D#] - 40-column catalog
- [x] CHAIN pn [,@#] [,S#] [,D#] - run program keeping variables
- [x] CLOSE [pn] - close file(s)
- [x] CREATE pn [,Ttype] [,S#] [,D#] - create file/directory
- [x] DELETE pn [,S#] [,D#] - delete file
- [x] EXEC pn [,S#] [,D#] - execute text file commands
- [x] FLUSH [pn] - flush file buffers
- [x] LOCK pn [,S#] [,D#] - lock file
- [x] UNLOCK pn [,S#] [,D#] - unlock file
- [x] OPEN pn [,Llength] [,S#] [,D#] - open file
- [x] POSITION pn [,Rrecord#] [,Bbyte#] - position in file
- [x] PREFIX [pn] [,S#] [,D#] - set/show prefix
- [x] READ pn [,Rrecord#] [,Bbyte#] - prepare file for reading
- [x] RENAME pn1,pn2 [,S#] [,D#] - rename file
- [x] ProDOS RESTORE pn [,S#] [,D#] - load variables from file
- [x] ProDOS STORE pn [,S#] [,D#] - save variables to file
- [x] WRITE pn [,Rrecord#] - prepare file for writing

## Built-in Functions

### Mathematical Functions
- [x] ABS(expression) - absolute value
- [x] ATN(expression) - arctangent (radians)
- [x] COS(expression) - cosine (radians)
- [x] EXP(expression) - e^x
- [x] INT(expression) - truncate to integer
- [x] LOG(expression) - natural logarithm
- [x] RND(expression) - random number
- [x] SGN(expression) - sign (-1, 0, or 1)
- [x] SIN(expression) - sine (radians)
- [x] SQR(expression) - square root
- [x] TAN(expression) - tangent (radians)

### String Functions
- [x] ASC(string) - ASCII code of first character
- [x] CHR$(code) - character from ASCII code
- [x] LEFT$(string, n) - leftmost n characters
- [x] LEN(string) - string length
- [x] MID$(string, start [,length]) - substring
- [x] RIGHT$(string, n) - rightmost n characters
- [x] STR$(expression) - convert number to string
- [x] VAL(string) - convert string to number

### System Functions
- [x] FRE(dummy) - free memory (returns placeholder)
- [x] PDL(n) - game paddle position (returns 0)
- [x] PEEK(address) - read memory byte
- [x] POS(dummy) - cursor horizontal position
- [x] SCRN(x,y) - read screen pixel color
- [x] USR(expression) - user machine code call (returns 0)

## Error Handling

### Standard Applesoft Error Codes
- [x] Error code 0: NEXT without FOR
- [x] Error code 16: Syntax
- [x] Error code 22: RETURN without GOSUB
- [x] Error code 42: Out of data
- [x] Error code 53: Illegal quantity
- [x] Error code 69: Overflow
- [x] Error code 77: Out of memory
- [x] Error code 90: Undefined statement
- [x] Error code 107: Bad subscript
- [x] Error code 120: Redimensioned array
- [x] Error code 133: Division by zero
- [x] Error code 163: Type mismatch
- [x] Error code 176: String too long
- [x] Error code 191: Formula too complex
- [x] Error code 224: Undefined function
- [x] Error code 254: Bad response to INPUT statement
- [x] Error code 255: CONTROL-C interrupt attempted

### ProDOS Error Codes
- [x] Error codes 2-21: ProDOS-specific errors
- [x] RANGE ERROR (Code 2)
- [x] NO DEVICE CONNECTED (Code 3)
- [x] WRITE PROTECTED (Code 4)
- [x] END OF DATA (Code 5)
- [x] PATH NOT FOUND (Codes 6-7)
- [x] I/O ERROR (Code 8)
- [x] DISK FULL (Code 9)
- [x] FILE LOCKED (Code 10)
- [x] INVALID OPTION (Code 11)
- [x] NO BUFFERS AVAILABLE (Code 12)
- [x] FILE TYPE MISMATCH (Code 13)
- [x] PROGRAM TOO LARGE (Code 14)
- [x] NOT DIRECT COMMAND (Code 15)
- [x] DIRECTORY FULL (Code 17)
- [x] FILE NOT OPEN (Code 18)
- [x] DUPLICATE FILENAME (Code 19)
- [x] FILE BUSY (Code 20)
- [x] FILE(S) STILL OPEN (Code 21)

### Error Handling Memory Locations
- [x] Location 216: Error handler control (POKE 216,0 to restore)
- [x] Location 218: Error line number (low byte)
- [x] Location 219: Error line number (high byte)
- [x] Location 222: Error code (0-255)

## PEEK/POKE/CALL Address Support

### Keyboard and Input
- [x] PEEK(-16384)/PEEK(49152) - last key pressed
- [x] POKE -16368/49168 - clear keyboard strobe
- [x] PEEK(-16287)/PEEK(49249) - button 0 (>127 if pressed)
- [x] PEEK(-16286)/PEEK(49250) - button 1 (>127 if pressed)
- [x] PEEK(-16285)/PEEK(49251) - button 2 (>127 if pressed)

### Memory Pointers
- [x] PEEK(105) - LOMEM pointer (low byte)
- [x] PEEK(106) - LOMEM pointer (high byte)
- [x] PEEK(115) - HIMEM pointer (low byte)
- [x] PEEK(116) - HIMEM pointer (high byte)
- [x] PEEK(218) - error line number (low byte)
- [x] PEEK(219) - error line number (high byte)
- [x] PEEK(222) - error code

### Display Control
- [x] PEEK(37) - cursor vertical position (0-23)
- [x] POKE 32 - text window left edge
- [x] POKE 33 - text window width
- [x] POKE 34 - text window top (0-23)
- [x] POKE 37 - cursor vertical position
- [x] POKE -16368/49168 - clear keyboard strobe
- [x] POKE -16304/49232 - switch to graphics without clearing
- [x] POKE -16303/49233 - full screen graphics control
- [x] POKE -16302/49234 - graphics mode control
- [x] POKE -16301/49235 - mixed graphics/text mode
- [x] POKE -16300/49236 - text mode control
- [x] POKE -16299/49237 - undocumented display control
- [x] POKE -16298/49238 - undocumented display control
- [x] POKE -16297/49239 - undocumented display control

### Annunciator Outputs
- [x] POKE -16296/49240 - turn off annunciator 0
- [x] POKE -16295/49241 - turn on annunciator 0
- [x] POKE -16294/49242 - turn off annunciator 1
- [x] POKE -16293/49243 - turn on annunciator 1
- [x] POKE -16292/49244 - turn off annunciator 2
- [x] POKE -16291/49245 - turn on annunciator 2
- [x] POKE -16290/49246 - turn off annunciator 3
- [x] POKE -16289/49247 - turn on annunciator 3

### Graphics Memory
- [x] POKE 103 - hi-res page pointer (low byte)
- [x] POKE 104 - hi-res page pointer (high byte)
- [x] POKE 16384 - page-specific graphics control
- [x] POKE 24576 - page-specific graphics control

### Shape Tables
- [x] POKE 232 - shape table pointer (low byte)
- [x] POKE 233 - shape table pointer (high byte)

### Error Control
- [x] POKE 216,0 - restore normal error handling

### System Calls (CALL addresses)
- [x] CALL -3288/62248 - stack cleanup routine
- [x] CALL -3086/62450 - clear hi-res page to black
- [x] CALL -3082/62454 - clear hi-res to last HPLOT color
- [x] CALL -1998/63538 - BKGND (background color)
- [x] CALL -958/64578 - clear from cursor to bottom-right
- [x] CALL -936/64600 - HOME (clear screen, home cursor)
- [x] CALL -922/64614 - line feed
- [x] CALL -912/64624 - scroll text window up
- [x] CALL -868/64668 - CLREOL (clear to end of line)
- [x] CALL -151/65385 - enter Monitor
- [x] CALL 768 - common user ML routine location (page 3)
- [x] CALL 1002 - restore ProDOS connection (ProDOS only)

## Implementation Notes

### Variable Names
- Variable names are significant to first 2 characters (case-insensitive)
- Exception: `FN` names preserve 3 characters (e.g., `FNx`)
- `$` suffix denotes string variables
- `%` suffix denotes integer variables (clamped to 16-bit signed range)

### Arrays
- Auto-dimension to size 10 per dimension if undeclared
- Expression subscripts supported
- Bounds checking yields "BAD SUBSCRIPT" error
- Shared get/set access path

### DATA/READ/RESTORE
- DATA collected pre-run before program execution
- READ consumes sequentially across control flow
- RESTORE can optionally target a specific line number
- READ on exhausted data raises "OUT OF DATA" error

### Graphics
- GR/HIRES configure scaled overlay window
- Shape table support with default shapes (triangle, square)
- User-defined shapes via SHLOAD (DATA-driven format)
- DRAW/XDRAW apply rotation (ROTATE) and scale (SCALE) transforms
- XDRAW implements XOR behavior (toggling pixels)
- SCRN(x,y) queries pixel buffer and returns color
- **No on-screen rendering yet** - graphics buffer only

### Memory Model
- LOMEM/HIMEM boundaries enforced for PEEK/POKE/WAIT
- Simple in-memory model for PEEK/POKE operations
- CALL is currently a no-op
- USR(addr) stub returns 0

### Output Behavior
- CHR$(7) emits terminal bell (`\a`) and flushes immediately
- POS reports current column using:
  - ANSI query on POSIX systems
  - Windows console API on Windows
  - Falls back to 0 if unavailable

### Device Control
- PR#n and IN#n store device slots but keep screen/keyboard active
- No actual device redirection implemented yet

### File I/O
- Basic LOAD/SAVE/CATALOG implemented
- RECALL/STORE for arrays implemented
- ProDOS extended file operations not yet implemented

## Testing

- Test suite location: `tests/` directory
- Tests are `.bas` programs executed via `ctest --test-dir build`
- Run tests: `ctest --test-dir build --output-on-failure`
- Always add `.bas` test programs for new features

## Build and Development

- Build: `cmake -S . -B build && cmake --build build`
- VS Code task: `build-msbasic` builds the `msbasic` target
- Version: Generated from `src/version.h.in` using `git describe`
- REPL: `./build/msbasic` (interactive mode)
- Run script: `./build/msbasic path/to/program.bas`

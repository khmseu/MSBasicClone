# Applesoft BASIC Language Features

_Extracted from Apple II Applesoft BASIC Programmer's Reference Manual_

## Arrays

**Description:** Arrays are collections of variables accessed by subscripts. They must be declared with DIM before use (except for arrays with subscripts 0-10). Arrays can have up to 88 dimensions.

---

## Cassette Operations

**Description:** SAVE and LOAD without filenames work with cassette tape. SHLOAD loads shape tables from cassette. STORE saves array data to cassette, and RECALL loads array data from cassette.

---

## Colors

**Description:** Low-resolution graphics supports 16 colors (0-15): black, magenta, dark blue, purple, dark green, grey, medium blue, light blue, brown, orange, grey, pink, light green, yellow, aqua, white.

---

## Comments

**Description:** Comments can be added to programs using the REM statement. Everything after REM on a line is ignored during execution.

---

## Conditional Execution

**Description:** IF...THEN statements execute code only if a condition is true. Multiple statements can follow THEN, separated by colons.

---

## Control Stack

**Description:** The control stack stores return addresses for GOSUB and loop information for FOR...NEXT. Maximum nesting: 10 FOR loops, 24 subroutine calls, 36 parenthesis levels.

---

## Error Handling

**Description:** ONERR GOTO allows programs to handle errors gracefully by branching to an error-handling routine when an error occurs. RESUME returns control to the statement that caused the error.

---

## Graphics Modes

**Description:** TEXT mode: 24 lines of 40 columns. GR mode: 40x40 low-resolution color graphics with 4 lines of text. HGR mode: 280x160 high-resolution graphics with 4 lines of text. HGR2 mode: 280x192 full-screen high-resolution graphics.

---

## Immediate vs Deferred Execution

**Description:** Commands typed without a line number execute immediately. Commands preceded by a line number are stored as part of the program and execute when the program runs.

---

## Input/Output

**Description:** INPUT reads from keyboard or peripheral, GET reads single characters without waiting for RETURN, PRINT writes to screen or peripheral, READ/DATA provide internal data storage.

---

## Line Numbers

**Description:** Each program line must begin with a line number (0-63999). Lines are executed in numerical order unless control statements alter the flow.

---

## Loops

**Description:** FOR...NEXT loops repeat a block of statements a specified number of times. The loop variable takes on successive values from a start value to an end value, optionally incrementing by a STEP amount.

---

## Machine Language Interface

**Description:** PEEK reads memory locations, POKE writes to memory, CALL executes machine language subroutines, USR calls machine language functions that return values.

---

## Memory Management

**Description:** HIMEM: and LOMEM: control the memory boundaries for programs and variables. This is important when using graphics pages or loading machine language routines.

---

## Multiple Statements

**Description:** Multiple statements can be placed on one line by separating them with colons (:).

---

## Numeric Format

**Description:** Numbers are stored in 5-byte floating point format with 9-digit precision. Integer variables use 2 bytes and are faster for whole number arithmetic.

---

## Operators

**Description:** Arithmetic operators: + (addition), - (subtraction), \* (multiplication), / (division), ^ (exponentiation)
Relational operators: = (equal), <> (not equal), < (less than), > (greater than), <= (less than or equal), >= (greater than or equal)
Logical operators: AND, OR, NOT
String operator: + (concatenation)

---

## Precision and Range

**Description:** Floating point numbers range from ±1E-38 to ±1E+38 with 9 decimal digits of precision. Integers range from -32768 to +32767.

---

## Random Numbers

**Description:** The RND function generates pseudo-random numbers. RND(1) returns a new random number 0≤x<1. RND(0) repeats the last value. RND with negative argument reseeds the generator.

---

## Reserved Words

**Description:** Applesoft has reserved words (keywords) that cannot be used as variable names. However, they can be embedded in longer variable names (e.g., GOTO is reserved, but GOTOX is valid).

---

## Shape Tables

**Description:** Shape tables store vector graphics that can be drawn with DRAW and XDRAW. ROT= sets rotation angle, SCALE= sets size.

---

## String Handling

**Description:** Strings can contain 0-255 characters. String operations include concatenation (+), substring extraction (LEFT$, RIGHT$, MID$), length (LEN), and conversion to/from numeric values (STR$, VAL).

---

## Subroutines

**Description:** Subroutines are blocks of code that can be called from multiple places using GOSUB. They return to the calling location with RETURN.

---

## User-Defined Functions

**Description:** DEF FN allows defining custom functions that can be called with FN. Functions can have one parameter and return a calculated value.

---

## Variables

**Description:** Applesoft supports three types of variables: real (numeric), integer (numeric with % suffix), and string (with $ suffix). Variable names can be 1-238 characters long but only the first two are significant.

---

## ProDOS Features

The following features are provided by ProDOS and ProDOS BASIC.SYSTEM.

### Binary Files

**Description:** Files containing machine language programs or raw data. Commands: BLOAD (load), BRUN (load and execute), BSAVE (save). Store load address and length information.

---

### Chaining Programs

**Description:** CHAIN allows running multiple BASIC programs in sequence while preserving variables between programs. Useful for large programs that exceed memory or for menu systems.

---

### Date and Time

**Description:** ProDOS tracks file creation and modification dates/times if a clock/calendar card is installed. Date/time values stored in specific memory locations accessible via PEEK/POKE.

---

### Directory Files

**Description:** Special files that contain lists of other files and their locations. Can be nested up to 64 levels deep. Each directory (except volume directory) can hold entries until disk is full.

---

### EXEC Files

**Description:** Text files containing ProDOS and BASIC commands that execute as if typed from the keyboard. Created with PRINT to a text file. Useful for automating tasks and creating startup sequences.

---

### Error Handling with ProDOS

**Description:** ProDOS error codes are stored in specific memory locations that can be read with PEEK. ONERR GOTO can trap ProDOS errors. Error codes include: FILE NOT FOUND, DISK FULL, FILE LOCKED, etc.

---

### Fast File Access

**Description:** ProDOS provides fast file housekeeping routines accessed via FRE function. These routines offer optimized performance for common file operations.

---

### File Buffers

**Description:** ProDOS uses memory buffers as temporary storage when reading from or writing to files. Maximum of 8 files can be open simultaneously due to buffer limitations.

---

### File Operations

**Description:** ProDOS provides commands for file management including CAT/CATALOG (list files), CREATE (make directories/files), RENAME (change names), DELETE (remove files), LOCK/UNLOCK (protect/unprotect files).

---

### File Types

**Description:** ProDOS supports multiple file types: TXT (text), BAS (Applesoft program), VAR (Applesoft variables), BIN (binary), DIR (directory), SYS (system), REL (relocatable code), and user-defined types.

---

### Memory Configuration

**Description:** ProDOS uses high memory for its code and buffers. HIMEM: may need adjustment when using graphics pages or loading machine language. BASIC.SYSTEM automatically sets appropriate HIMEM.

---

### Partial Pathnames

**Description:** A partial pathname does not begin with a slash. ProDOS forms a complete pathname by adding the current prefix to the partial pathname. This simplifies file access within a working directory.

---

### Pathnames

**Description:** A pathname is a series of filenames preceded and separated by slashes (/) that indicates the path from the volume directory to a file. Example: /VOLUME/DIRECTORY/FILENAME. Maximum 64 characters including slashes.

---

### Prefix

**Description:** The prefix is a default pathname that ProDOS automatically adds to partial pathnames. Set with PREFIX command. Maximum 64 characters. Simplifies access to files in a working directory.

---

### ProDOS BASIC.SYSTEM

**Description:** The system program that provides ProDOS commands in BASIC. Must be present on startup disk. Loads into memory at startup and provides the interface between BASIC and ProDOS.

---

### Random-Access Text Files

**Description:** Text files partitioned into fixed-length records. Each record can be individually read or written at any position. Specify record length when opening. Useful for databases and indexed data.

---

### Sequential Text Files

**Description:** Text files organized as a sequence of fields. Each field is a string of characters terminated by a carriage return. Best for data that will be stored and retrieved sequentially.

---

### Slot and Drive Options

**Description:** Commands support [,S#] for slot number (1-7) and [,D#] for drive number (1-2). These options provide DOS 3.3 compatibility and allow accessing files without knowing the volume name.

---

### Variable Storage

**Description:** STORE saves all current BASIC variables to a VAR file. RESTORE loads variables from a VAR file. Variables can be preserved between program runs or shared between programs.

---

### Volume Directory

**Description:** The main directory on a ProDOS disk. Automatically created when formatting. Can contain up to 51 files. Volume name appears with leading slash (/) in pathnames.

---

# Applesoft BASIC Error Messages

This document lists all error messages found in the Apple II Applesoft BASIC Programmer's Reference Manual Volume 1.

## Error Handling

When an error occurs in Applesoft BASIC:

1. The error code is stored in memory location 222 (read with PEEK(222))
2. The error line number is stored in locations 218 (low byte) and 219 (high byte)
3. Normal error handling displays an error message and halts the program
4. Custom error handling can be implemented using ONERR GOTO statement

## Error Codes (Table 3-1)

These are the official Applesoft BASIC error codes as listed in the manual.

| Code | Error Message | Description |
|------|---------------|-------------|
| 0 | NEXT without FOR | Occurs when a NEXT statement is encountered without a matching FOR statement |
| 16 | Syntax | A statement or expression doesn't conform to Applesoft's syntax rules. There are a myriad of possible causes for this error, such as a missing parenthesis, illegal character, or incorrect punctuation. Often results from a simple typing error. |
| 22 | RETURN without GOSUB | A RETURN statement was encountered without a corresponding GOSUB having been executed. This error often occurs when control accidentally branches into a subroutine via a GOTD statement, or "falls into" a subroutine because there is no END or GOTD statement at the end of the program segment preceding the subroutine. |
| 42 | Out of data | A READ statement was executed after all DATA statements in the program had already been read. A READ statement may have been executed more times than intended (for example, in an infinite loop), or one or more DATA statements may have been inadvertently omitted. Sometimes caused by accidentally leaving out a RESTORE statement. |
| 53 | Illegal quantity | The argument supplied to a statement or function was out of the allowed range. This error can be caused by - a negative array subscript (for example, LET A (-1) = 0) - LOG with a negative or zero argument SQR with a negative argument A B with A negative and B not an integer use of LEFT$, MID$, RIGHT$, WAIT, PEEK, POKE, CALL, TAB, SPC, ON...GOTO, ON...GOSUB, or any of the graphics statements or functions with an improper argument |
| 69 | Overflow | The result of an arithmetic calculation was too large to be represented in Applesoft's internal number format. |
| 77 | Out of memory | Any of the following can cause this error: - Program too large - Too many variables FOR loops nested more than 10 levels deep - Subroutine calls nested more than 24 levels deep - Parentheses nested more than 36 levels deep - Too complicated an expression - Attempt to set LOMEM : too high - Attempt to set L O M E M : lower than present value - Attempt to set HIMEM: too low |
| 90 | Undefined statement | An attempt was made to transfer control, via GOTO, GOSUB, or IF...THEN, to a nonexistent line number. Common causes include accidentally deleting a line, changing a line number without changing references from other lines accordingly, and simple typing errors. |
| 107 | Bad subscript | A reference was made to an array element that is outside the dimensions of the array. This error can occur if the wrong number of dimensions is used in an array reference: for instance, when A has been defined by |
| 120 | Redimensioned array | An attempt was made to define the same array twice in the same or different D I M statements. This error often occurs if an array has been referred to in a statement such as |
| 133 | Division by zero | An attempt was made to divide by zero; division by zero is mathematically undefined. Often occurs when a variable is used in an arithmetic expression before begin given a value (all numeric variables initially have the value zero). To debug, examine the divisor of the expression where the error occurred to see why it unexpectedly has a zero value. |
| 163 | Type mismatch | The left side of an assignment statement was a numeric variable and the right side was a string, or vice versa; or a function that expected a string argument was given a numeric one or vice versa. Often caused by inadvertently leaving out the dollar sign (\$) in a string variable or function name. |
| 176 | String too long | Occurs when a string exceeds the maximum length of 255 characters |
| 191 | Formula too complex | Occurs when an expression is too complex for BASIC to evaluate |
| 224 | Undefined function | A reference was made to a function that had never been defined. May occur when you type something like FN L (X) when you meant to type FN I (X); that is, a simple case of mistaken identifier. |
| 254 | Bad response to INPUT statement | Occurs when the user enters invalid data in response to an INPUT statement |
| 255 | CONTROL-C interrupt attempted | Occurs when the user presses CONTROL-C to interrupt program execution |

## ProDOS Error Codes

These error codes are specific to ProDOS commands and operations.

| Code | Error Message | Most Common Cause |
|------|---------------|-------------------|
| 2 | RANGE ERROR | Command option too small or large |
| 3 | NO DEVICE CONNECTED | No device found in specified slot |
| 4 | WRITE PROTECTED | Write-protect tab on disk |
| 5 | END OF DATA | Read beyond end of file or record |
| 6 | PATH NOT FOUND | No file with indicated pathname |
| 7 | PATH NOT FOUND | No file with indicated pathname |
| 8 | I/O ERROR | Door open, or disk not formatted |
| 9 | DISK FULL | Too many files on a disk |
| 10 | FILE LOCKED | Attempt to write to a locked file |
| 11 | INVALID OPTION | Option inappropriate for command |
| 12 | NO BUFFERS AVAILABLE | Memory full, file can't be opened |
| 13 | FILE TYPE MISMATCH | Disk file wrong type for command |
| 14 | PROGRAM TOO LARGE | Apple II's memory too small (CHAIN) |
| 15 | NOT DIRECT COMMAND | Command must be in a program |
| 16 | SYNTAX ERROR | Bad filename, option, or comma |
| 17 | DIRECTORY FULL | Volume directory has 51 files |
| 18 | FILE NOT OPEN | Attempt to access a closed file |
| 19 | DUPLICATE FILENAME | RENAME, CREATE name already used |
| 20 | FILE BUSY | File already open |
| 21 | FILE(S) STILL OPEN | Last program didn't close file(s) |

## Using Error Codes in Programs

To handle errors in your program:

```basic
10 ONERR GOTO 1000
...
1000 EC = PEEK(222): REM Get error code
1010 EL = PEEK(218) + PEEK(219) * 256: REM Get error line
1020 IF EC = 255 THEN PRINT "INTERRUPTED": RESUME
1030 PRINT "ERROR ";EC;" AT LINE ";EL: STOP
```

To restore normal error handling: `POKE 216,0`

### Error Handling Memory Locations

- **Location 222**: Contains the error code (0-255) when an error occurs
- **Location 218**: Low byte of the line number where error occurred
- **Location 219**: High byte of the line number where error occurred
- **Location 216**: POKE 216,0 restores normal error handling

## Detailed ProDOS Error Descriptions

### RANGE ERROR (Code 2)
Occurs when the value of a ProDOS command option is too large or too small. Note: The use of values outside the indicated ranges does not always cause the RANGE ERROR message. Any ProDOS command option that is less than 0 or greater than 65535 causes the SYNTAX ERROR message, not the RANGE ERROR message.

### NO DEVICE CONNECTED (Code 3)
Occurs when you specify a slot that doesn't contain a card; a slot that contains a card not connected to its device; or if there is no disk in the drive (some drives only).

### WRITE PROTECTED (Code 4)
Occurs when ProDOS attempts to store information on a disk, and the disk drive does not detect a write-enable notch or cutout on the disk's outer case. Remove the adhesive label over the write-enable notch, or cut a notch if one doesn't exist.

### END OF DATA (Code 5)
Occurs when you try to retrieve information from a portion of a text file where no information has ever been stored. Can be caused by:
- Too many successive INPUTs or an INPUT with too many variables
- Too many successive GETs
- The B# (Byte) or F# (Field) option in a READ or POSITION command is too large
- The R# (Record) option in a READ command specified a record that has not been written to

### PATH NOT FOUND (Code 6 or 7)
Occurs when a ProDOS command specifies a valid pathname that does not indicate an existing file, or when it specifies an invalid pathname. Common causes:
- Misspelled an element of the pathname
- Used a partial pathname that doesn't apply to the current prefix
- The specified file does not yet exist

### I/O ERROR (Code 8)
Occurs after an unsuccessful attempt to store data or retrieve data (ProDOS tries 96 times, then gives up). Can be caused by:
- The drive's door is open
- There is no disk in the disk drive
- The disk is not formatted
- The disk is incorrectly seated in the drive
- Specified a non-existent disk drive
- Trying to access a 13-sector disk (use MUFFIN to update to 16 sectors)

### DISK FULL (Code 9)
Occurs when ProDOS attempts to store information on a disk and finds that no more storage space is available. All files are closed, and ProDOS saved as much as it could.

### FILE LOCKED (Code 10)
Occurs when you try to APPEND, BSAVE, DELETE, RENAME, SAVE, STORE, or WRITE a locked file. Locked files are indicated with an asterisk (*) in the CATALOG display.

### INVALID OPTION (Code 11)
Occurs when you use an option that is either non-existent or that is inappropriate for the given command.

### NO BUFFERS AVAILABLE (Code 12)
When a file is opened, a 1K buffer in memory is assigned for temporary storage. There can be a maximum of eight files open at a time. This error occurs if a command is used when eight files are already open, or if there is not enough free memory for a buffer.

### FILE TYPE MISMATCH (Code 13)
Occurs when a ProDOS command expects to use one type of file, and the specified file is of another type. Correct file type combinations:
- CATALOG, PREFIX: Must be a directory file (DIR)
- LOAD, RUN, SAVE, CHAIN: Must be an Applesoft program file (BAS)
- RESTORE, STORE: Must be an Applesoft variable file (VAR)
- EXEC: Must be a text file (TXT)
- OPEN, APPEND: Must be a text file (TXT) unless Ttype is used
- BRUN: Must be a binary file (BIN)
- BLOAD, BSAVE: Must be a binary file (BIN) unless Ttype is used

### PROGRAM TOO LARGE (Code 14)
Occurs when a ProDOS command attempts to place a disk file into memory, and finds the available memory insufficient. Can be caused by CHAIN, LOAD, RESTORE, RUN, or - commands. You may have set HIMEM too low.

### NOT DIRECT COMMAND (Code 15)
Occurs when you try to use one of the text file commands APPEND, OPEN, POSITION, READ, or WRITE from immediate-execution mode. These commands can only be used from within PRINT statements in program lines.

### SYNTAX ERROR (Code 16)
ProDOS-specific: Occurs when ProDOS encounters a syntax error in a ProDOS command. Check for:
- Pathname with illegal characters
- Incorrect option symbol
- Missing option or separator (usually a comma)
- Option value less than 0 or greater than 65535

**Special case**: If all ProDOS commands cause SYNTAX ERROR, type `CALL 1002` to restore ProDOS connection.

### DIRECTORY FULL (Code 17)
A ProDOS volume directory file can hold up to 51 files. Save the file into another directory or onto another disk.

### FILE NOT OPEN (Code 18)
Occurs when a command is issued that can only act upon an open file (POSITION, READ, WRITE). You must open a file before using these commands.

### DUPLICATE FILENAME (Code 19)
Occurs when you CREATE or RENAME a file using a pathname that already exists.

### FILE BUSY (Code 20)
Occurs when you CAT, CATALOG, DELETE, or RENAME a file that is already open. You must close a file before using one of these commands.

### FILE(S) STILL OPEN (Code 21)
Occurs when program execution is interrupted while one or more files are still open (for example, by another error or CONTROL-C). You must close all open files before you issue another LOAD or RUN statement.

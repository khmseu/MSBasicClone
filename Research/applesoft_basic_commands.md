# Applesoft BASIC Commands

_Extracted from Apple II Applesoft BASIC Programmer's Reference Manual_

## ProDOS Commands

The following commands are provided by ProDOS BASIC.SYSTEM and are available when running programs under ProDOS.

### - (DASH)

**Syntax:** `- pn [,S#] [,D#]`

**Description:** Run a BASIC program, machine-language program, EXEC file, or system program from a file without clearing variables from memory.

---

### APPEND

**Syntax:** `APPEND pn`

**Description:** Prepare a file for writing by positioning to the end of the file. New data is added to the end without overwriting existing content.

---

### BLOAD

**Syntax:** `BLOAD pn [,A#] [,S#] [,D#]`

**Description:** Load a binary file into memory without executing it. Optionally specify a load address with A#. If no address is specified, uses the file's stored load address.

---

### BRUN

**Syntax:** `BRUN pn [,A#] [,S#] [,D#]`

**Description:** Load a binary file into memory and execute it. Optionally specify a load address with A#.

---

### BSAVE

**Syntax:** `BSAVE pn,A#,L# [,S#] [,D#]`

**Description:** Save a portion of memory as a binary file. A# specifies the starting address and L# specifies the length in bytes.

---

### CAT

**Syntax:** `CAT [pn] [,S#] [,D#]`

**Description:** Display a 40-column wide list of files in a directory. Shows the file's name, type, size, and modified date.

---

### CATALOG

**Syntax:** `CATALOG [pn] [,S#] [,D#]`

**Description:** Display an 80-column wide list of files in a directory. Shows the same information as CAT plus the date the file was created, the logical length of the file, and some subtype information.

---

### CHAIN

**Syntax:** `CHAIN pn [,@#] [,S#] [,D#]`

**Description:** Run a BASIC program from a file without clearing the current variables from memory. Optionally specify a starting line number with @#.

---

### CLOSE

**Syntax:** `CLOSE [pn]`

**Description:** Write all unwritten data to the file and release the file buffers. If no pathname is specified, all open files are closed.

---

### CREATE

**Syntax:** `CREATE pn [,Ttype] [,S#] [,D#]`

**Description:** Create a new directory or other file type with a specified name and type. The default file type is directory (DIR).

---

### DELETE

**Syntax:** `DELETE pn [,S#] [,D#]`

**Description:** Remove a file from its directory. A deleted file cannot be recovered. The file must be unlocked before it can be deleted.

---

### EXEC

**Syntax:** `EXEC pn [,S#] [,D#]`

**Description:** Execute commands from a sequential text file as if they were typed at the keyboard. Press CONTROL-C to terminate EXEC mode.

---

### FLUSH

**Syntax:** `FLUSH [pn]`

**Description:** Write all unwritten data from memory buffers to the file without closing it. Ensures data integrity in case of power failure or program crash.

---

### LOCK

**Syntax:** `LOCK pn [,S#] [,D#]`

**Description:** Protect a file from being accidentally renamed, deleted, or altered. A locked file is indicated by an asterisk (*) in the catalog listing.

---

### OPEN

**Syntax:** `OPEN pn [,Llength] [,S#] [,D#]`

**Description:** Allocate space in memory for a file's buffers and set the file position pointer to the beginning. For random-access text files, Llength specifies the record length.

---

### POSITION

**Syntax:** `POSITION pn [,Rrecord#] [,Bbyte#]`

**Description:** Move the file position pointer to a specific location in the file. For random-access files, specify record and byte position.

---

### PREFIX

**Syntax:** `PREFIX [pn] [,S#] [,D#]`

**Description:** Set the prefix (default pathname) to indicate a specific directory. When used without options, displays the current prefix. The prefix is automatically added to partial pathnames.

---

### READ

**Syntax:** `READ pn [,Rrecord#] [,Bbyte#]`

**Description:** Prepare a file for reading. For random-access files, optionally specify the record number (R#) and byte position (B#). Subsequent INPUT statements read from the file.

---

### RENAME

**Syntax:** `RENAME pn1,pn2 [,S#] [,D#]`

**Description:** Change a file's name from pn1 to pn2 within the same directory. Cannot be used to move a file to a different directory.

---

### RESTORE

**Syntax:** `RESTORE pn [,S#] [,D#]`

**Description:** Clear the current BASIC variables and load a new set of variables from a variable file (type VAR) that was previously saved with STORE. Note: This is different from the standard Applesoft RESTORE command for DATA statements.

---

### STORE

**Syntax:** `STORE pn [,S#] [,D#]`

**Description:** Save the current BASIC variables to a variable file (type VAR) for later retrieval with RESTORE.

---

### UNLOCK

**Syntax:** `UNLOCK pn [,S#] [,D#]`

**Description:** Unprotect a locked file so it can be renamed, deleted, or altered.

---

### WRITE

**Syntax:** `WRITE pn [,Rrecord#]`

**Description:** Prepare a file for writing. For random-access files, optionally specify the record number. Subsequent PRINT statements write to the file.

---

## Standard Applesoft Commands

## CALL

**Syntax:** `CALL address`

**Description:** Executes a machine-language subroutine at the specified decimal memory address.

---

## CLEAR

**Syntax:** `CLEAR`

**Description:** Resets the values of all numeric variables to 0 and those of all string variables to the null string; also resets Applesoft's internal control information to its initial state. Has no effect on the program lines in memory.

---

## COLOR=

**Syntax:** `COLOR= expression`

**Description:** Sets the display color for plotting low-resolution graphics. Value should be from 0 to 15.

---

## CONT

**Syntax:** `CONT`

**Description:** Resumes program execution after it has been interrupted.

---

## DATA

**Syntax:** `DATA value[,value...]`

**Description:** Creates a list of items for use by READ statements. Items can be numbers or strings.

---

## DEF FN

**Syntax:** `DEF FN name(parameter) = expression`

**Description:** Defines a user function that can be used in expressions.

---

## DEL

**Syntax:** `DEL line1[,line2]`

**Description:** Deletes one or more lines from the program in memory.

---

## DIM

**Syntax:** `DIM array(size[,size...]) [,array(size[,size...])]...`

**Description:** Defines and allocates space for one or more arrays. Specifies the maximum subscripts for each dimension.

---

## DRAW

**Syntax:** `DRAW shape_number [AT x,y]`

**Description:** Draws a shape from the shape table at the current or specified location.

---

## END

**Syntax:** `END`

**Description:** Terminates execution of the program and returns control to the user. No message is displayed.

---

## FLASH

**Syntax:** `FLASH`

**Description:** Causes subsequently printed text characters to flash alternately between inverse and normal video.

---

## FOR

**Syntax:** `FOR variable = start TO end [STEP increment]`

**Description:** Marks the beginning of a loop, identifies the index variable, and gives the variable's starting and ending values and optionally the step increment.

---

## GET

**Syntax:** `GET variable`

**Description:** Reads a single character from the current input device without waiting for RETURN to be pressed. The character is not displayed on the screen.

---

## GOSUB

**Syntax:** `GOSUB line_number`

**Description:** Executes a subroutine beginning at the designated line number. Control returns to the statement following GOSUB when a RETURN is encountered.

---

## GOTO

**Syntax:** `GOTO line_number`

**Description:** Sends control unconditionally to the designated line number.

---

## GR

**Syntax:** `GR`

**Description:** Converts the display to 40 rows of low-resolution graphics with four lines of text at the bottom. The screen is cleared to black, the cursor is moved to the beginning of the last line, and the low-resolution display color is set to black.

---

## HCOLOR=

**Syntax:** `HCOLOR= expression`

**Description:** Sets the display color for high-resolution graphics.

---

## HGR

**Syntax:** `HGR`

**Description:** Converts the display to 160 rows of high-resolution graphics with four lines for text at the bottom. The screen is cleared to black and page 1 of high-resolution graphics is displayed.

---

## HGR2

**Syntax:** `HGR2`

**Description:** Converts the display to full-screen (192 rows) high-resolution graphics with no text. The screen is cleared to black and page 2 of high-resolution graphics is displayed.

---

## HIMEM:

**Syntax:** `HIMEM: address`

**Description:** Sets the highest memory location available for use by Applesoft programs and variables.

---

## HLIN

**Syntax:** `HLIN x1,x2 AT y`

**Description:** Draws a horizontal line from column x1 to column x2 in row y on the low-resolution graphics screen.

---

## HOME

**Syntax:** `HOME`

**Description:** Clears the currently defined text window and sends the cursor to the top-left corner of the window.

---

## HPLOT

**Syntax:** `HPLOT x,y [TO x,y]...`

**Description:** Plots a point or line on the high-resolution graphics screen in the current high-resolution display color.

---

## HTAB

**Syntax:** `HTAB column`

**Description:** Moves the cursor to the specified horizontal position (column) on the current line of the text window.

---

## IF

**Syntax:** `IF condition THEN statement[:statement...] or IF condition THEN line_number`

**Description:** Executes or skips one or more statements, depending on the truth of a stated condition.

---

## IN#

**Syntax:** `IN# slot`

**Description:** Designates the peripheral device in the specified slot as the current input device.

---

## INPUT

**Syntax:** `INPUT ["prompt";] variable[,variable...]`

**Description:** Reads a line of input from the current input device and assigns values to the specified variables. Can include an optional prompting message.

---

## INVERSE

**Syntax:** `INVERSE`

**Description:** Causes subsequently printed text characters to appear in inverse video (black on white).

---

## LIST

**Syntax:** `LIST [start[,end]]`

**Description:** Displays on the screen all or part of the program currently in memory. Can list entire program, a range of lines, or a single line.

---

## LOAD

**Syntax:** `LOAD filename`

**Description:** Reads an Applesoft program from a file on disk or tape into the computer's memory for execution or editing.

---

## LOMEM:

**Syntax:** `LOMEM: address`

**Description:** Sets the lowest memory location available for use by Applesoft arrays.

---

## NEW

**Syntax:** `NEW`

**Description:** Clears the current program from memory, resets the values of all numeric variables to 0 and those of all string variables to the null string, and prepares Applesoft to accept a new program.

---

## NEXT

**Syntax:** `NEXT [variable[,variable...]]`

**Description:** Marks the end of a loop and causes the loop to be repeated for the next value of the index variable.

---

## NORMAL

**Syntax:** `NORMAL`

**Description:** Cancels inverse video and causes subsequently printed text to appear in normal video (white on black).

---

## NOTRACE

**Syntax:** `NOTRACE`

**Description:** Turns off line number tracing.

---

## ON

**Syntax:** `ON expression GOTO|GOSUB line_number[,line_number...]`

**Description:** Branches to one of several line numbers based on the value of an expression.

---

## ONERR

**Syntax:** `ONERR GOTO line_number`

**Description:** Establishes an error-handling routine. When an error occurs, control branches to the specified line number.

---

## PLOT

**Syntax:** `PLOT x,y`

**Description:** Plots a single block of the current display color at a specified position on the low-resolution graphics screen.

---

## POKE

**Syntax:** `POKE address,value`

**Description:** Stores a value (0-255) at a specified location in memory.

---

## POP

**Syntax:** `POP`

**Description:** Removes the most recent entry from the control stack, allowing exit from a subroutine without executing a RETURN.

---

## PR#

**Syntax:** `PR# slot`

**Description:** Designates the peripheral device in the specified slot as the current output device.

---

## PRINT

**Syntax:** `PRINT [expression[;|,expression]...]`

**Description:** Writes a line of output to the current output device. Expressions can be separated by semicolons (for tight spacing) or commas (to advance to next tab position).

---

## READ

**Syntax:** `READ variable[,variable...]`

**Description:** Reads values from DATA statements in the body of the program and assigns them to the specified variables.

---

## RECALL

**Syntax:** `RECALL arrayname[%]`

**Description:** Reads values into an array from a tape cassette. Used to load array data that was previously saved with STORE.

---

## REM

**Syntax:** `REM comment`

**Description:** Includes remarks in the body of a program for the benefit of a human reader. Everything following REM on the same line is ignored during execution.

---

## RESTORE

**Syntax:** `RESTORE`

**Description:** Resets the DATA pointer to the beginning of the program so that the next READ will start from the first DATA statement.

---

## RESUME

**Syntax:** `RESUME`

**Description:** Returns control from an error-handling routine to the statement that caused the error.

---

## RETURN

**Syntax:** `RETURN`

**Description:** Returns control from a subroutine to the statement following the GOSUB that called the subroutine.

---

## ROT=

**Syntax:** `ROT= angle`

**Description:** Sets the rotation angle for shape drawing (0-63).

---

## RUN

**Syntax:** `RUN [line_number] or RUN filename`

**Description:** Executes the program currently in memory. If no line number is given, execution begins at the beginning of the program. Can also load and execute a program from disk.

---

## SAVE

**Syntax:** `SAVE filename`

**Description:** Writes the program currently in memory onto a disk or tape cassette for future use.

---

## SCALE=

**Syntax:** `SCALE= size`

**Description:** Sets the scale factor for shape drawing (1-255).

---

## SHLOAD

**Syntax:** `SHLOAD`

**Description:** Loads a shape table from tape cassette.

---

## SPEED=

**Syntax:** `SPEED= value`

**Description:** Sets the speed at which characters are displayed on the screen (0-255).

---

## STOP

**Syntax:** `STOP`

**Description:** Terminates execution of the program and returns control to the user. Displays a message identifying the program line in which the STOP statement appears.

---

## STORE

**Syntax:** `STORE arrayname[%]`

**Description:** Stores values from an array onto a tape cassette. The array data can later be loaded back with RECALL.

---

## TEXT

**Syntax:** `TEXT`

**Description:** Converts the display to 24 lines of text, with the cursor positioned at the beginning of the bottom line.

---

## TRACE

**Syntax:** `TRACE`

**Description:** Causes each line number to be displayed as it is executed, for debugging purposes.

---

## VLIN

**Syntax:** `VLIN y1,y2 AT x`

**Description:** Draws a vertical line from row y1 to row y2 in column x on the low-resolution graphics screen.

---

## VTAB

**Syntax:** `VTAB row`

**Description:** Moves the cursor to the specified vertical position (row) within the text window.

---

## WAIT

**Syntax:** `WAIT address,mask1[,mask2]`

**Description:** Suspends program execution until a specified bit pattern appears at a specified memory location.

---

## XDRAW

**Syntax:** `XDRAW shape_number [AT x,y]`

**Description:** Exclusive-OR draws a shape from the shape table at the current or specified location.

---

# PEEK, POKE, and CALL Addresses

This document lists all PEEK, POKE, and CALL addresses found in the Apple II Applesoft BASIC Programmer's Reference Manual Volume 1.

## Overview

These low-level commands provide direct access to the Apple II's memory and ROM routines:

- **PEEK(address)** - Reads and returns the byte value (0-255) at a memory location
- **POKE address,value** - Writes a byte value (0-255) to a memory location
- **CALL address** - Executes a machine language subroutine at the specified address

Negative addresses are automatically converted by adding 65536 to get the positive equivalent.

## PEEK Addresses

PEEK is used to read a value from a memory location. The function returns the byte value (0-255) stored at the specified address.

### PEEK(-32768) = PEEK(32768) = PEEK($8000)

**Usage examples:**

- PEEK (-1) is equivalent to PEEK (65535) PEEK (-32768) is equivalent to PEEK (32768)

### PEEK(-16287) = PEEK(49249) = PEEK($C061)

**Purpose:** Hand control 0 button - value > 127 if pressed

**Usage examples:**

- between PDL calls. Reading the hand control buttons Historical Note: The function name PDL stands for &quot;paddle,&quot; which in turn is short for &quot;game paddle,&quot; an older name for the Apple Ile&#x27;s hand

### PEEK(-16286) = PEEK(49250) = PEEK($C062)

**Purpose:** Hand control 1 button - value > 127 if pressed

**Usage examples:**

- controls. PEEK function: see Section 7.1.1 The buttons on the hand controls can be read with the function calls PEEK (-16287) —yields a value &gt; 127 if button on hand control 0 is being

### PEEK(-16285) = PEEK(49251) = PEEK($C063)

**Purpose:** Hand control 2 button - value > 127 if pressed

**Usage examples:**

- pressed, &lt;= 127 if not PEEK (-16286) —yields a value &gt; 127 if button on hand control 1 is being

### PEEK(-1) = PEEK(65535) = PEEK($FFFF)

**Usage examples:**

- PEEK（-16384） is equivalent to PEEK（49152） PEEK (-1) is equivalent to PEEK (65535)

### PEEK(105) = PEEK($69)

**Purpose:** LOMEM pointer (low byte) - start of variable storage

**Usage examples:**

- Helpful Hint: The current value of LOMEM is stored in decimal memory locations 105 and 106; to obtain that value, use the expression PEEK (106) * 256 + PEEK (105)

### PEEK(106) = PEEK($6A)

**Purpose:** LOMEM pointer (high byte) - start of variable storage

**Usage examples:**

- Helpful Hint: The current value of LOMEM is stored in decimal memory locations 105 and 106; to obtain that value, use the expression PEEK (106) * 256 + PEEK (105)

### PEEK(109) = PEEK($6D)

**Usage examples:**

- IF（PEEK(112)*256+PEEK(111)) -（PEEK(110)*256+PEEK(109)）>2 THEN

### PEEK(110) = PEEK($6E)

**Usage examples:**

- IF（PEEK(112)*256+PEEK(111)) -（PEEK(110)*256+PEEK(109)）>2 THEN

### PEEK(111) = PEEK($6F)

**Usage examples:**

- IF（PEEK(112)*256+PEEK(111)) -（PEEK(110)*256+PEEK(109)）>2 THEN

### PEEK(112) = PEEK($70)

**Usage examples:**

- IF（PEEK(112)*256+PEEK(111)) -（PEEK(110)*256+PEEK(109)）>2 THEN

### PEEK(115) = PEEK($73)

**Purpose:** HIMEM pointer (low byte) - top of available memory

**Usage examples:**

- High-Resolution Graphics,&quot; for details. Helpful Hint: The current value of HIMEM : is stored in decimal mem- ory locations 115 and 116; to obtain that value, use the expression

### PEEK(116) = PEEK($74)

**Purpose:** HIMEM pointer (high byte) - top of available memory

**Usage examples:**

- High-Resolution Graphics,&quot; for details. Helpful Hint: The current value of HIMEM : is stored in decimal mem- ory locations 115 and 116; to obtain that value, use the expression

### PEEK(218) = PEEK($DA)

**Purpose:** Error line number (low byte) - combined with location 219 gives the line where error occurred

**Usage examples:**

- More Peeking: In the program above, the general error message displayed in line 21550 would be more useful if it included the line number where the error occurred as well as the error code itself. Through the magic of the PEEK function, the following

### PEEK(219) = PEEK($DB)

**Purpose:** Error line number (high byte) - combined with location 218 gives the line where error occurred

**Usage examples:**

- More Peeking: In the program above, the general error message displayed in line 21550 would be more useful if it included the line number where the error occurred as well as the error code itself. Through the magic of the PEEK function, the following

### PEEK(222) = PEEK($DE)

**Purpose:** Error code - stores the error code (0-255) when an error occurs

**Usage examples:**

- 21500 LET EC = PEEK (222) —get error code
- 21500 POKE 216,0 -restore normal error handling 21505 LET EC = PEEK (222)

### PEEK(32768) = PEEK($8000)

**Usage examples:**

- PEEK (-1) is equivalent to PEEK (65535) PEEK (-32768) is equivalent to PEEK (32768)

### PEEK(65535) = PEEK($FFFF)

**Usage examples:**

- PEEK（-16384） is equivalent to PEEK（49152） PEEK (-1) is equivalent to PEEK (65535)


### PEEK(-16384) = PEEK(49152) = PEEK($C000)

**Purpose:** Reads the last character typed from the keyboard. If the high-order bit of this location is 1 (PEEK yields a result $>127$ ), then a new character has been typed since the last POKE to address - 16368 (see below); subtracting 128 from the value received gives the ASCII code for the character typed. 

**Source:** Volume 2

### PEEK(37) = PEEK($25)

**Purpose:** Yields the current vertical position of the cursor, which will be a number between 0 and 23.

**Source:** Volume 2

## POKE Addresses

POKE is used to write a value to a memory location. The statement takes an address and a value (0-255) to store at that address.

### POKE -65502 = POKE 34 = POKE($22)

**Usage examples:**

- POKE -65502,8 - POKE -16384, 0 is equivalent to POKE 49152, 0 POKE -32768, 112 is equivalent to POKE 32768, 112

### POKE -32768 = POKE 32768 = POKE($8000)

**Usage examples:**

- POKE -32768,112 - POKE -16384, 0 is equivalent to POKE 49152, 0 POKE -32768, 112 is equivalent to POKE 32768, 112

### POKE -16384 = POKE 49152 = POKE($C000)

**Usage examples:**

- POKE -16384,0 - If POKE is given a negative target address, it adds 65536 (2 to the 16th power) to obtain an equivalent positive address. For example, POKE -16384, 0 is equivalent to POKE 49152, 0

### POKE -16303 = POKE 49233 = POKE($C051)

**Purpose:** Full screen graphics control

**Usage examples:**

- POKE -16303,0 - POKE -16300, 0 POKE -16303, 0
- POKE -16303,0 - The TEXT statement will return to text display with the text window set to the full screen and the cursor at the bottom of the screen. To turn off high-resolution graphics and return to text display with the text window and cursor int

### POKE -16302 = POKE 49234 = POKE($C052)

**Purpose:** Graphics mode control

**Usage examples:**

- POKE -16302,0 - POKE -16302, 0 POKE ADDR，（2\*D1+3\*D2）/（U- $\mathcal{V}$ ）
- POKE -16302,0 - POKE statement: see Section 7.1.2 POKE -16302,0

### POKE -16301 = POKE 49235 = POKE($C053)

**Purpose:** Mixed graphics/text mode control

**Usage examples:**

- POKE -16301,0 - This statement will change the bottom four lines of text to high-resolution graphics. To return to mixed graphics and text, use POKE -16301, 0
- POKE -16301,0 - After executing an HGR2 statement, you can convert the display to mixed graphics and text (a 280-by-160 grid with four lines of text at the bottom) with the statement POKE -16301, 0

### POKE -16300 = POKE 49236 = POKE($C054)

**Purpose:** Text mode control

**Usage examples:**

- POKE -16300,0 - The TEXT statement will return to text display with the text window set to the full screen and the cursor at the bottom of the screen. To turn off high-resolution graphics and return to text display with the text window and cursor int

### POKE 32

**Purpose:** Sets the left edge of the text window to the value specified by expression L.

**Source:** Volume 2

### POKE 33

**Purpose:** Sets the width of the text window (number of characters per line) to the value specified by expression $\mathbb{W}$ .

**Source:** Volume 2

### POKE 34 = POKE($22)

**Purpose:** Text window top - sets the top line of the text window (0-23)

**Usage examples:**

- POKE 34,8 - POKE -16384, 0 is equivalent to POKE 49152, 0 POKE -32768, 112 is equivalent to POKE 32768, 112
- POKE 34,8 - expression, separated from the first by a comma, gives the value to be stored into that location. For example, stores value 8 into location 34
- POKE 34,8 - # 7.1.2 The POKE Statement POKE -16302, 0
- 100 PRINT: POKE 34,6 - (From ProDOS manual)

**Source:** Volume 2, ProDOS Manual

### POKE 103 = POKE($67)

**Purpose:** High-res graphics page pointer (low byte)

**Usage examples:**

- POKE 103,1 - These statements will start program and variable storage above high-resolution page 2, beginning at address 24576 (hexadecimal $6000): POKE 103, 1
- POKE 103,1 - A third method, probably the best for long programs with lots of variables, is to use the wizardry of the POKE statement to change the start, instead of the end, of Applesoft's program storage space. The following statements will start p

### POKE 104 = POKE($68)

**Purpose:** High-res graphics page pointer (high byte)

**Usage examples:**

- POKE 104,96 - POKE 103, 1 POKE 104,96
- POKE 104,64 - POKE 103, 1 POKE 104,64

### POKE 216

**Purpose:** Error handling control - POKE 216,0 restores normal error handling

**Usage examples:**

- POKE 216,0 - Error Handling One use of this technique is to prevent your program from hanging or falling into the Monitor in case an error occurs in the error-handling routine itself. You can do this by restoring normal error handling with POKE 216,0
- POKE 216,0 - 21540 RESUME and resume program This program also illustrates another application of POKE 216,0. Notice that if the error is anything other than a CONTROL-C interrupt (code 255), the IF...THEN test in line 21510 sends control directly to
- POKE 216,0 - 21500 POKE 216,0 -restore normal error handling 21505 LET EC = PEEK (222)

### POKE 232 = POKE($E8)

**Purpose:** Shape table pointer (low byte)

**Usage examples:**

- POKE 232,252 - dress is not stored automatically into the special addresses where Applesoft looks for them, so you (or your program) will have to do that
- POKE 232,252 - Now that you have your shape table in memory, you have to tell Applesoft where to find it. Applesoft looks for the table's starting address in hexadecimal locations E8 (low-order byte) and E9 (high-order byte), so you have to arrange s

### POKE 233 = POKE($E9)

**Purpose:** Shape table pointer (high byte)

**Usage examples:**

- POKE 233,29 - dress is not stored automatically into the special addresses where Applesoft looks for them, so you (or your program) will have to do that
- POKE 233,29 - Now that you have your shape table in memory, you have to tell Applesoft where to find it. Applesoft looks for the table's starting address in hexadecimal locations E8 (low-order byte) and E9 (high-order byte), so you have to arrange so

### POKE 16384 = POKE($4000)

**Usage examples:**

- POKE 16384,0 - POKE 104,64 POKE 16384,0

### POKE 24576 = POKE($6000)

**Usage examples:**

- POKE 24576,0 - POKE 104,96 POKE 24576,0

### POKE 32768 = POKE($8000)

**Usage examples:**

- POKE 32768,112 - POKE -16384, 0 is equivalent to POKE 49152, 0 POKE -32768, 112 is equivalent to POKE 32768, 112

### POKE 49152 = POKE($C000)

**Usage examples:**

- POKE 49152,0 - If POKE is given a negative target address, it adds 65536 (2 to the 16th power) to obtain an equivalent positive address. For example, POKE -16384, 0 is equivalent to POKE 49152, 0


### POKE -16368 = POKE 49168 = POKE($C010)

**Purpose:** Clears the high-order bit of location - 16384 (see above) to prepare for reading another keyboard character.

**Source:** Volume 2

### POKE -16304 = POKE 49232 = POKE($C050)

**Purpose:** Switches the display from full-screen text to graphics without clearing the graphics screen.

**Source:** Volume 2

### POKE -16299 = POKE 49237 = POKE($C055)

**Purpose:** POKE -16299,0

**Source:** Volume 2

### POKE -16298 = POKE 49238 = POKE($C056)

**Purpose:** POKE -16298,0

**Source:** Volume 2

### POKE -16297 = POKE 49239 = POKE($C057)

**Purpose:** POKE -16297,0

**Source:** Volume 2

### POKE -16296 = POKE 49240 = POKE($C058)

**Purpose:** Turns off annunciator output 0 (hand control connector, pin 15).

**Source:** Volume 2

### POKE -16295 = POKE 49241 = POKE($C059)

**Purpose:** Turns on annunciator output 0 (hand control connector, pin 15).

**Source:** Volume 2

### POKE -16294 = POKE 49242 = POKE($C05A)

**Purpose:** Turns off annunciator output 1 (hand control connector, pin 14).

**Source:** Volume 2

### POKE -16293 = POKE 49243 = POKE($C05B)

**Purpose:** Turns on annunciator output 1 (hand control connector, pin 14).

**Source:** Volume 2

### POKE -16292 = POKE 49244 = POKE($C05C)

**Purpose:** Turns off annunciator output 2 (hand control connector, pin 13).

**Source:** Volume 2

### POKE -16291 = POKE 49245 = POKE($C05D)

**Purpose:** Turns on annunciator output 2 (hand control connector, pin 13).

**Source:** Volume 2

### POKE -16290 = POKE 49246 = POKE($C05E)

**Purpose:** Turns off annunciator output 3 (hand control connector, pin 12).

**Source:** Volume 2

### POKE -16289 = POKE 49247 = POKE($C05F)

**Purpose:** Turns on annunciator output 3 (hand control connector, pin 12).

**Source:** Volume 2

### POKE 32 = POKE($20)

**Purpose:** Sets the left edge of the text window to the value specified by expression L.

**Source:** Volume 2

### POKE 33 = POKE($21)

**Purpose:** Sets the width of the text window (number of characters per line) to the value specified by expression $\mathbb{W}$ .

**Source:** Volume 2

### POKE 37 = POKE($25)

**Purpose:** Moves the cursor to the vertical position specified by expression $\square \mathcal{V}$ , which is interpreted relative to the top of the screen, not the top of the text window.

**Source:** Volume 2

### POKE 216 = POKE($D8)

**Purpose:** Error handling control - POKE 216,0 restores normal error handling after ONERR GOTO

**Usage examples:**

- POKE 216,0 - Error Handling One use of this technique is to prevent your program from hanging or falling into the Monitor in case an error occurs in the error-handling routine itself. You can do this by restoring normal error handling with POKE 216,0
- POKE 216,0 - 21540 RESUME and resume program This program also illustrates another application of POKE 216,0. Notice that if the error is anything other than a CONTROL-C interrupt (code 255), the IF...THEN test in line 21510 sends control directly to
- POKE 216,0 - 21500 POKE 216,0 -restore normal error handling 21505 LET EC = PEEK (222)

**Source:** ProDOS Manual

### POKE 218 = POKE($DA)

**Purpose:** Error line number (low byte) - combined with location 219 gives the line where error occurred

**Usage examples:**

- Used to retrieve the line number where an error occurred: L = PEEK(218) + PEEK(219) * 256

**Source:** ProDOS Manual

### POKE 219 = POKE($DB)

**Purpose:** Error line number (high byte) - combined with location 218 gives the line where error occurred

**Usage examples:**

- Used to retrieve the line number where an error occurred: L = PEEK(218) + PEEK(219) * 256

**Source:** ProDOS Manual

### POKE 222 = POKE($DE)

**Purpose:** Error code storage - stores the error code (0-255) when an error occurs in ONERR GOTO

**Usage examples:**

- E = PEEK(222) - Gets the error code after an error occurs
- 21500 LET EC = PEEK (222) —get error code
- 21500 POKE 216,0 -restore normal error handling 21505 LET EC = PEEK (222)
- 200 PRINT "ERROR#" PEEK (222) DETECTED"

**Source:** ProDOS Manual

## CALL Addresses

CALL is used to execute a machine language subroutine at the specified address. These are built-in ROM routines or user-defined machine language programs.

### CALL -3288 = CALL 62248 = CALL($F318)

**Purpose:** Stack cleanup routine

**Usage examples:**

- later on. To avert a global catastrophe, always &quot;clean up&quot; the stack by uttering the magical incantation

### CALL -1998 = CALL 63538 = CALL($F832)

**Purpose:** BKGND - sets background color for text

**Usage examples:**

- This statement will change the bottom four lines of text to eight rows of colored blocks. To clear these rows to black, add high-resolution page 2: see Section 6.2.2
- is equivalent to

### CALL -936 = CALL 64600 = CALL($FC58)

**Purpose:** HOME routine - clears screen and homes cursor

**Usage examples:**

- CALL ROUTINE (J)
- If CALL is given a negative target address, it adds 65536 (2 to the 16th power) to obtain an equivalent positive address. For example, is equivalent to

### CALL -868 = CALL 64668 = CALL($FC9C)

**Purpose:** CLREOL - clears to end of line

**Usage examples:**

- 510 PRINT "Please enter the WEIGHT - a number plus an 0 (for ounces) or a P (for pounds) - and press the RETURN Key: " ; -prompting message to tell user what information to type and how to type it 520 CALL -868 clear to end of line; useful to erase a
- is equivalent to

### CALL -151 = CALL 65385 = CALL($FF69)

**Purpose:** Monitor - enters the Apple II Monitor

**Usage examples:**

- Here is a trivial example showing the use of the USR function. The machine-language routine shown here simply takes the argument value it receives and multiplies it by 8: CALL -151

### CALL 37 = CALL($25)

**Usage examples:**

- built-in arithmetic 38 call 37, 38, 45

### CALL 61 = CALL($3D)

**Usage examples:**

- subroutine(s) 10,61ff,171,229 250,269,270,276 execution 220
- plus sign $(+)$ 36,84,105,295 point of call 61, 64

### CALL 768 = CALL($300)

**Purpose:** Common location for user machine language routines (page 3)

**Usage examples:**

- The following subroutine uses the CALL statement to call the machine-language routine at address 768. (Again, this subroutine can be located anywhere in your Applesoft program, not necessarily at line number 63000.) 63000 REM CALL &quot;INPUT ANYTHIN

### CALL 54915 = CALL($D683)

**Usage examples:**

- # 7.1.3 The CALL Statement

### CALL 63538 = CALL($F832)

**Usage examples:**

- is equivalent to If the target address is not in the range -65535 to +65535, the program will halt with the message

### CALL 64600 = CALL($FC58)

**Usage examples:**

- is equivalent to

### CALL 64668 = CALL($FC9C)

**Usage examples:**

- subroutine at the designated address; when the subroutine is finished, execution continues with the statement following the CALL. For example, — executes machine-language subroutine beginning at address 64668
- is equivalent to


### CALL -3086 = CALL 62450 = CALL($F3F2)

**Purpose:** Cleared the current high-resolution page to black. (Applesoft remembers which page you used last, regardless of the switch settings.)

**Source:** Volume 2

### CALL -3082 = CALL 62454 = CALL($F3F6)

**Purpose:** Cleared the current high-resolution page to the color most recently used in an HPLD statement.

**Source:** Volume 2

### CALL -958 = CALL 64578 = CALL($FC42)

**Purpose:** Clears all characters inside the text window from the current cursor position to the bottom-right corner.

**Source:** Volume 2

### CALL -922 = CALL 64614 = CALL($FC66)

**Purpose:** Issues a line feed character, causing the cursor to move down one line without changing its horizontal position.

**Source:** Volume 2

### CALL -912 = CALL 64624 = CALL($FC70)

**Purpose:** Scrolls all text within the text window up one line.

**Source:** Volume 2

### CALL 1002 = CALL($3EA)

**Purpose:** Restores ProDOS connection to BASIC input/output when ProDOS becomes disconnected

**Usage examples:**

- If all ProDOS commands inexplicably cause the SYNTAX ERROR message, ProDOS is not started up or is "disconnected" from input and output. To restore, type CALL 1002 from BASIC (from the Monitor, press CONTROL-C to enter BASIC, then type CALL 1002). If this doesn't work, start up the disk again.

**Source:** ProDOS Manual


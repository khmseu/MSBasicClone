# Applesoft BASIC Functions

_Extracted from Apple II Applesoft BASIC Programmer's Reference Manual_

## ABS

**Syntax:** `ABS(expression)`

**Description:** Returns the absolute value of the expression.

---

## ASC

**Syntax:** `ASC(string)`

**Description:** Returns the ASCII code (0-255) of the first character of the string.

---

## ATN

**Syntax:** `ATN(expression)`

**Description:** Returns the arctangent of the expression (in radians), result is in range -π/2 to +π/2.

---

## CHR$

**Syntax:** `CHR$(code)`

**Description:** Returns a one-character string containing the character with the specified ASCII code (0-255).

---

## COS

**Syntax:** `COS(expression)`

**Description:** Returns the cosine of the expression (in radians).

---

## EXP

**Syntax:** `EXP(expression)`

**Description:** Returns e raised to the power of the expression.

---

## FN

**Syntax:** `FN name(argument)`

**Description:** Calls a user-defined function that was defined with DEF FN.

---

## FRE

**Syntax:** `FRE(dummy)`

**Description:** Returns the amount of free memory available for programs and variables. The argument is ignored but required. With ProDOS, FRE can also be used to access fast file housekeeping routines when called with specific negative values.

---

## INT

**Syntax:** `INT(expression)`

**Description:** Returns the largest integer less than or equal to the expression (truncates toward negative infinity).

---

## LEFT$

**Syntax:** `LEFT$(string, n)`

**Description:** Returns the leftmost n characters of the string.

---

## LEN

**Syntax:** `LEN(string)`

**Description:** Returns the length of the string (number of characters).

---

## LOG

**Syntax:** `LOG(expression)`

**Description:** Returns the natural logarithm (base e) of the expression.

---

## MID$

**Syntax:** `MID$(string, start [,length])`

**Description:** Returns a substring starting at position start. If length is specified, returns that many characters; otherwise returns to end of string.

---

## PDL

**Syntax:** `PDL(n)`

**Description:** Reads the position (0-255) of game paddle n (0-3).

---

## PEEK

**Syntax:** `PEEK(address)`

**Description:** Returns the contents (0-255) of a specified location in memory.

---

## POS

**Syntax:** `POS(dummy)`

**Description:** Returns the current horizontal cursor position within the text window. The argument is ignored but required.

---

## RIGHT$

**Syntax:** `RIGHT$(string, n)`

**Description:** Returns the rightmost n characters of the string.

---

## RND

**Syntax:** `RND(expression)`

**Description:** Returns a pseudo-random number. If expression > 0, returns a random number between 0 and 1. If expression = 0, repeats the last random number. If expression < 0, reseeds the random number generator.

---

## SCRN

**Syntax:** `SCRN(x, y)`

**Description:** Returns the color code of the low-resolution graphics block at column x, row y.

---

## SGN

**Syntax:** `SGN(expression)`

**Description:** Returns the sign of the expression: -1 if negative, 0 if zero, +1 if positive.

---

## SIN

**Syntax:** `SIN(expression)`

**Description:** Returns the sine of the expression (in radians).

---

## SPC

**Syntax:** `SPC(n)`

**Description:** Used in PRINT statements to output n spaces.

---

## SQR

**Syntax:** `SQR(expression)`

**Description:** Returns the square root of the expression.

---

## STR$

**Syntax:** `STR$(expression)`

**Description:** Converts a numeric expression to a string representation.

---

## TAB

**Syntax:** `TAB(column)`

**Description:** Used in PRINT statements to advance the cursor to the specified column. Returns spaces needed to reach that column.

---

## TAN

**Syntax:** `TAN(expression)`

**Description:** Returns the tangent of the expression (in radians).

---

## USR

**Syntax:** `USR(expression)`

**Description:** Executes a user-supplied machine-language function routine, passing it a specified argument.

---

## VAL

**Syntax:** `VAL(string)`

**Description:** Converts a string to a numeric value. Reads digits from left to right until a non-numeric character is encountered.

---

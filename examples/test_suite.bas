10 REM COMPREHENSIVE APPLESOFT BASIC TEST
20 REM Tests all major features of the interpreter
30 PRINT "========================================="
40 PRINT "APPLESOFT BASIC INTERPRETER TEST SUITE"
50 PRINT "========================================="
60 PRINT
70 REM Test 1: Basic math
80 PRINT "TEST 1: ARITHMETIC OPERATORS"
90 PRINT "  2 + 3 = "; 2 + 3
100 PRINT "  10 - 4 = "; 10 - 4
110 PRINT "  6 * 7 = "; 6 * 7
120 PRINT "  20 / 4 = "; 20 / 4
130 PRINT "  2 ^ 10 = "; 2 ^ 10
140 PRINT "  17 MOD 5 = "; 17 MOD 5
150 PRINT
160 REM Test 2: Variables
170 PRINT "TEST 2: VARIABLES"
180 LET A = 10
190 LET B = 20
200 LET C$ = "HELLO"
210 PRINT "  A = "; A
220 PRINT "  B = "; B
230 PRINT "  C$ = "; C$
240 PRINT
250 REM Test 3: Conditional logic
260 PRINT "TEST 3: IF/THEN/ELSE"
270 IF A < B THEN PRINT "  A is less than B"
280 IF A = B THEN PRINT "  A equals B"
290 IF A > B THEN PRINT "  A is greater than B" ELSE PRINT "  A is not greater than B"
300 PRINT
310 REM Test 4: Loops
320 PRINT "TEST 4: FOR/NEXT LOOPS"
330 PRINT "  Counting 1 to 5:"
340 FOR I = 1 TO 5
350 PRINT "    ";
355 PRINT I
360 NEXT I
370 PRINT
380 PRINT "  Counting by 2 from 0 to 10:"
390 FOR J = 0 TO 10 STEP 2
400 PRINT "    ";
405 PRINT J
410 NEXT J
420 PRINT
430 PRINT
440 REM Test 5: Math functions
450 PRINT "TEST 5: MATH FUNCTIONS"
460 PRINT "  SQR(16) = "; SQR(16)
470 PRINT "  ABS(-5) = "; ABS(-5)
480 PRINT "  INT(3.7) = "; INT(3.7)
490 PRINT "  SGN(-10) = "; SGN(-10)
500 PRINT
510 REM Test 6: String functions
520 PRINT "TEST 6: STRING FUNCTIONS"
530 LET S$ = "BASIC"
540 PRINT "  String: "; S$
550 PRINT "  LEN = "; LEN(S$)
560 PRINT "  LEFT$(3) = "; LEFT$(S$, 3)
570 PRINT "  RIGHT$(2) = "; RIGHT$(S$, 2)
580 PRINT "  MID$(2,3) = "; MID$(S$, 2, 3)
590 PRINT
600 REM Test 7: Subroutines
610 PRINT "TEST 7: SUBROUTINES"
620 GOSUB 1000
630 PRINT "  Back from subroutine"
640 PRINT
650 PRINT "========================================="
660 PRINT "ALL TESTS COMPLETED SUCCESSFULLY!"
670 PRINT "========================================="
680 END
1000 REM Subroutine
1010 PRINT "  Inside subroutine"
1020 RETURN

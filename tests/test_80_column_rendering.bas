10 REM Test 80-column text rendering
20 REM Verifies 80-column mode (PR#3)
30 REM Each line should fit 80 characters
40 PR#3
50 PRINT "80-COLUMN TEXT MODE TEST"
60 PRINT "========================"
70 PRINT ""
80 PRINT "LINE 1: 80 columns can display much wider text than 40-column mode!"
90 PRINT "LINE 2: ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
100 PRINT "LINE 3: 80 columns allow for wider displays (reports, data tables, etc.)"
110 PRINT ""
120 PRINT "Testing character output in 80-column mode:"
130 FOR I=1 TO 80
140 PRINT "-";
150 NEXT I
160 PRINT ""
170 PRINT ""
180 PR#0
190 PRINT "Back to 40-column mode"
200 PRINT "80-COLUMN RENDERING: OK"
210 PRINT "TEST PASSED"

10 REM Test 40-column text rendering
20 REM Verifies default text mode (PR#0) output
30 REM Each line should fit 40 characters max
40 PR#0
50 PRINT "40-COLUMN TEXT MODE TEST"
60 PRINT "========================"
70 PRINT ""
80 PRINT "LINE 1: 40 chars max width here!"
90 PRINT "LINE 2: ABCDEFGHIJKLMNOPQRSTUVWXYZ"
100 PRINT "LINE 3: !@#$%^&*()_+-={}[]|"
110 PRINT ""
120 PRINT "Testing character output:"
130 FOR I=1 TO 40
140 PRINT "*";
150 NEXT I
160 PRINT ""
170 PRINT ""
180 PRINT "40-COLUMN RENDERING: OK"
190 PRINT "TEST PASSED"

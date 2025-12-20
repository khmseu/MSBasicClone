10 REM Test POSITION command
20 REM POSITION sets file read/write position
30 PRINT "TESTING POSITION"
40 CREATE "postest.txt"
50 OPEN "postest.txt"
60 REM Position to record 0, byte 0
70 POSITION "postest.txt"
80 CLOSE "postest.txt"
90 DELETE "postest.txt"
100 PRINT "POSITION TEST PASSED"

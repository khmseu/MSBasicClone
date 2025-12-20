10 REM Test OPEN and CLOSE commands
20 PRINT "TESTING OPEN/CLOSE"
30 CREATE "testfile.txt"
40 OPEN "testfile.txt"
50 CLOSE "testfile.txt"
60 PRINT "OPEN/CLOSE TEST PASSED"
70 DELETE "testfile.txt"

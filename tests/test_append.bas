10 REM Test APPEND command
20 REM APPEND prepares a file for appending data
30 PRINT "TESTING APPEND"
40 CREATE "appendtest.txt"
50 APPEND "appendtest.txt"
60 CLOSE "appendtest.txt"
70 DELETE "appendtest.txt"
80 PRINT "APPEND TEST PASSED"

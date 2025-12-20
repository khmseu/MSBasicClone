10 REM Test FLUSH command
20 REM FLUSH forces file buffers to disk
30 PRINT "TESTING FLUSH"
40 CREATE "flushtest.txt"
50 OPEN "flushtest.txt"
60 FLUSH "flushtest.txt"
70 CLOSE "flushtest.txt"
80 DELETE "flushtest.txt"
90 PRINT "FLUSH TEST PASSED"

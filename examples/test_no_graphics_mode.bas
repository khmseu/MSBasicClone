10 REM This test must be run with --no-graphics flag
20 REM It verifies that graphics commands fail in no-graphics mode
30 ONERR GOTO 100
40 GR
50 PRINT "ERROR: GR should have failed!"
60 END
100 REM Error handler
110 IF PEEK(222) <> 0 THEN PRINT "OK: Graphics command blocked (error code "; PEEK(222); ")": END
120 PRINT "ERROR: No error code set": END


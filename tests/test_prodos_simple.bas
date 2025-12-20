10 REM TEST PRODOS STORE AND RESTORE COMMANDS - SIMPLE VERSION
20 REM Set up some variables
30 A = 42
40 B = 3.14159
50 D$ = "HELLO WORLD"
60 REM Store variables to file
70 STORE "test_vars2.dat"
80 PRINT "VARIABLES STORED"
90 REM Clear variables
100 A = 0
110 B = 0
120 D$ = ""
130 PRINT "VARIABLES CLEARED: A=";A;" B=";B;" D$=";D$
140 REM Restore variables from file
150 RESTORE "test_vars2.dat"
160 PRINT "VARIABLES RESTORED: A=";A;" B=";B;" D$=";D$
170 PRINT "TEST COMPLETED"

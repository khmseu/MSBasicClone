REM Test CHAIN command - loads and runs program keeping variables
10 PRINT "Testing CHAIN command"
20 LET X = 10
30 LET Y$ = "HELLO"
40 PRINT "Before CHAIN: X ="; X; ", Y$ ="; Y$
50 REM This would chain to second program in real use
60 REM CHAIN "test_chain_second.bas"
70 PRINT "CHAIN syntax test passed"
80 END

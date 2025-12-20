10 REM TEST SHLOAD WITH BINARY FILE
20 REM This test verifies that SHLOAD can read a shape table file
30 REM Create a simple test shape table file using BSAVE
40 REM Shape table format: numshapes, offset1_lo, offset1_hi, ...
50 PRINT "CREATING TEST SHAPE TABLE FILE"
60 REM Allocate memory at address 8192 for shape table
70 POKE 8192, 1
80 POKE 8193, 7
90 POKE 8194, 0
100 REM Add a simple shape (just a few bytes)
110 POKE 8195, 4
120 POKE 8196, 0
130 BSAVE "test_shape.tbl", A8192, L5
140 PRINT "TEST SHAPE TABLE FILE CREATED"
150 REM Now try to load it
160 SHLOAD "test_shape.tbl"
170 PRINT "SHAPE TABLE LOADED SUCCESSFULLY"
180 REM Check that shape table pointer was set
190 P1 = PEEK(232)
200 P2 = PEEK(233)
210 PRINT "SHAPE TABLE POINTER: ";P1;",";P2
220 PRINT "TEST COMPLETED"

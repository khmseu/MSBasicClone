REM Test ProDOS commands: DELETE, RENAME, PREFIX, CREATE, CAT
10 PRINT "Testing ProDOS commands"
20 REM Create a test file
30 CREATE "test_file.txt"
40 PRINT "Created test file"
50 REM List files
60 CAT
70 REM Show current prefix
80 PREFIX
90 REM Create another file for rename test
100 CREATE "old_name.txt"
110 PRINT "Created old_name.txt"
120 REM Rename the file
130 RENAME "old_name.txt", "new_name.txt"
140 PRINT "Renamed file"
150 REM List files again
160 CAT
170 REM Delete the test files
180 DELETE "test_file.txt"
190 DELETE "new_name.txt"
200 PRINT "Deleted test files"
210 PRINT "ProDOS commands test complete"
220 END

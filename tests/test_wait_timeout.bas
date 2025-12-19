10 REM WAIT with optional timeout should exit without condition true
20 POKE 4096,0
30 WAIT 4096,1,10
40 IF PEEK(4096)<>0 THEN 1/0
50 END

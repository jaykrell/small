3.exe: 3.c
 -del 3.exe 3.obj 4.exe
 cl /Zi /Gy /MD 3.c /link msvcrt.lib /incremental:no
 3.exe
 dir *.exe
 test.cmd

test:
 test.cmd

clean:
 del 3.exe 4.exe 3.obj


4.exe: 4.c
 -del 4.exe 4.obj 5.exe
 cl /Ox /Zi /Gy /MD 4.c /link msvcrt.lib /incremental:no
 4.exe
 dir *.exe

test:
 test.cmd

clean:
 del 4.exe 5.exe 4.obj


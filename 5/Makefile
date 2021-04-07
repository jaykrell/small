5.exe: 5.c
 -del 5.exe 5.obj 6.exe
 cl /Zi /MD 5.c /link msvcrt.lib /incremental:no
 5.exe
 dir *.exe

test:
 test.cmd

clean:
 del 5.exe 6.exe 5.obj *.pdb


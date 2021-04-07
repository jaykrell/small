6.exe: 6.c
 -del 6.exe 6.obj 7.exe
 cl /Zi /MD 6.c /link msvcrt.lib /incremental:no
 6.exe
 dir *.exe

test:
 test.cmd

clean:
 del 6.exe 7.exe 6.obj *.pdb


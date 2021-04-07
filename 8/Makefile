8.exe: 8.cpp app.asm
 -del *.exe *.obj *.pdb
 ml64 -c app.asm
 cl /Zi /MD 8.cpp /link msvcrt.lib /incremental:no app.obj kernel32.lib

test:
 test.cmd

clean:
 -del *.exe *.obj *.pdb


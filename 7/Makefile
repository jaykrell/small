7.exe: 7.c
 -del *.exe *.obj *.pdb
 cl /Zi /MD $? /link msvcrt.lib /incremental:no
 $@
 dir *.exe

test:
 test.cmd

clean:
 -del *.exe *.obj *.pdb


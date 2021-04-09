12.exe: 12.cpp app.asm
 ml64 -c -Zi app.asm
 cl /Zi /MD 12.cpp /link /incremental:no app.obj && 12.exe

clean:
 -del *.exe *.obj *.pdb *.dll *.ilk

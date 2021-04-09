13.exe: 13.cpp
 cl /Ox2 /Gy /GL /GS- /MD /Zi 13.cpp user32.lib /link /entry:Entry kernel32.lib /incremental:no /subsystem:windows /merge:.rdata=.text /opt:ref

clean:
 -del *.exe *.obj *.pdb *.dll *.ilk

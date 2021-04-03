setlocal

cl /Gy /MD 2.c /link msvcrt.lib /incremental:no 
2.exe
dir 2.exe
dir 3.exe

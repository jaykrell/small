setlocal

cl /Gy /O1 1.c /link /opt:ref,icf /subsystem:console /entry:Entry kernel32.lib /incremental:no /merge:.rdata=.text /merge:.data=.text /merge:.pdata=.text /align:16 /allowbind:no /fixed
1.exe
dir 1.exe

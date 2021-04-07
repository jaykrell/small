
extern __imp_WriteFile:ptr

public app
public app_end
public call_WriteFile

.data

s1 segment byte 'CODE' read write execute

app proc

; WriteFile(-11, "hello\n", 6, &app, 0) ; &hello instead of &app does not work
  push 0
  sub rsp, 32
  lea r9, app
  mov r8, 6
  lea rdx, hello
  mov rcx, -11
  call __imp_WriteFile ; patch later
call_WriteFile proc
call_WriteFile endp
  add rsp, 40
  ret

hello:
 db "hello", 10

app endp

s1 ends

app_end:

end

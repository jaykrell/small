
extern __imp_WriteFile:ptr
public app
public app_end
public call_WriteFile

.code

app proc

; WriteFile(-11, "hello\n", 6, &0, 0) ; &hello instead of &app does not work
  push 0
  mov r9, rsp
  sub rsp, 32
  mov r8, 6
  lea rdx, hello
  mov rcx, -11
  call [__imp_WriteFile] ; patch later
call_WriteFile proc
call_WriteFile endp
  add rsp, 40
  xor eax, eax
  ret

hello:
 db "hello", 10

app endp

app_end:

end

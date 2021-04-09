
extern __imp_puts:ptr
public app
public app_end
public call_puts

.code

app proc

; puts("hi")
  lea rcx, hello
  jmp [__imp_puts] ; patch later
call_puts proc
call_puts endp

hello:
 db "hi", 0

app endp

app_end:

end

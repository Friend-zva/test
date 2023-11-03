	.file	"sort.c"
	.text
	.p2align 4
	.globl	sort_array
	.type	sort_array, @function
sort_array:
.LFB0:
	.cfi_startproc
	endbr64
	leal	-1(%rsi), %r8d
	testl	%r8d, %r8d
	jle	.L1
	.p2align 4,,10
	.p2align 3
.L5:
	movq	%rdi, %rax
	xorl	%edx, %edx
	.p2align 4,,10
	.p2align 3
.L4:
	movq	(%rax), %rsi
	movq	8(%rax), %rcx
	cmpq	%rcx, %rsi
	jle	.L3
	movq	%rsi, 8(%rax)
	movq	%rcx, (%rax)
.L3:
	addl	$1, %edx
	addq	$8, %rax
	cmpl	%edx, %r8d
	jg	.L4
	subl	$1, %r8d
	jne	.L5
.L1:
	ret
	.cfi_endproc
.LFE0:
	.size	sort_array, .-sort_array
	.p2align 4
	.globl	swap_element
	.type	swap_element, @function
swap_element:
.LFB1:
	.cfi_startproc
	endbr64
	movq	(%rdi), %rax
	xorq	(%rsi), %rax
	movq	%rax, (%rdi)
	xorq	(%rsi), %rax
	movq	%rax, (%rsi)
	xorq	%rax, (%rdi)
	ret
	.cfi_endproc
.LFE1:
	.size	swap_element, .-swap_element
	.ident	"GCC: (Ubuntu 11.4.0-1ubuntu1~22.04) 11.4.0"
	.section	.note.GNU-stack,"",@progbits
	.section	.note.gnu.property,"a"
	.align 8
	.long	1f - 0f
	.long	4f - 1f
	.long	5
0:
	.string	"GNU"
1:
	.align 8
	.long	0xc0000002
	.long	3f - 2f
2:
	.long	0x3
3:
	.align 8
4:

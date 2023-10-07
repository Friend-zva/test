	.file	"sort.c"
	.text
	.align	2
	.p2align 4,,11
	.global	sort_array
	.type	sort_array, %function
sort_array:
.LFB0:
	.cfi_startproc
	sub	w5, w1, #1
	cmp	w5, 0
	ble	.L1
	add	x4, x0, w5, sxtw 3
	.p2align 3,,7
.L5:
	mov	x1, x0
	.p2align 3,,7
.L4:
	ldp	x2, x3, [x1]
	cmp	x2, x3
	ble	.L3
	stp	x3, x2, [x1]
.L3:
	add	x1, x1, 8
	cmp	x1, x4
	bne	.L4
	sub	x4, x4, #8
	subs	w5, w5, #1
	bne	.L5
.L1:
	ret
	.cfi_endproc
.LFE0:
	.size	sort_array, .-sort_array
	.align	2
	.p2align 4,,11
	.global	swap_element
	.type	swap_element, %function
swap_element:
.LFB1:
	.cfi_startproc
	ldr	x3, [x1]
	ldr	x2, [x0]
	eor	x2, x2, x3
	str	x2, [x0]
	ldr	x3, [x1]
	eor	x2, x2, x3
	str	x2, [x1]
	ldr	x1, [x0]
	eor	x1, x1, x2
	str	x1, [x0]
	ret
	.cfi_endproc
.LFE1:
	.size	swap_element, .-swap_element
	.ident	"GCC: (Ubuntu 12.3.0-1ubuntu1~23.04) 12.3.0"
	.section	.note.GNU-stack,"",@progbits

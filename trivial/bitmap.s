	.file	"bitmap.cpp"
	.text
	.section	.rodata
	.type	_ZStL19piecewise_construct, @object
	.size	_ZStL19piecewise_construct, 1
_ZStL19piecewise_construct:
	.zero	1
	.type	_ZStL13allocator_arg, @object
	.size	_ZStL13allocator_arg, 1
_ZStL13allocator_arg:
	.zero	1
	.type	_ZStL6ignore, @object
	.size	_ZStL6ignore, 1
_ZStL6ignore:
	.zero	1
	.section	.text._ZN6NAHeap7bit2idxEj,"axG",@progbits,_ZN6NAHeap7bit2idxEj,comdat
	.weak	_ZN6NAHeap7bit2idxEj
	.type	_ZN6NAHeap7bit2idxEj, @function
_ZN6NAHeap7bit2idxEj:
.LFB1348:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movl	%edi, -4(%rbp)
	subl	$1, -4(%rbp)
	movl	-4(%rbp), %eax
	shrl	%eax
	andl	$1431655765, %eax
	subl	%eax, -4(%rbp)
	movl	-4(%rbp), %eax
	shrl	$2, %eax
	andl	$858993459, %eax
	movl	%eax, %edx
	movl	-4(%rbp), %eax
	andl	$858993459, %eax
	addl	%edx, %eax
	movl	%eax, -4(%rbp)
	movl	-4(%rbp), %eax
	shrl	$4, %eax
	movl	%eax, %edx
	movl	-4(%rbp), %eax
	addl	%edx, %eax
	andl	$252645135, %eax
	movl	%eax, -4(%rbp)
	movl	-4(%rbp), %eax
	shrl	$8, %eax
	addl	%eax, -4(%rbp)
	movl	-4(%rbp), %eax
	shrl	$16, %eax
	addl	%eax, -4(%rbp)
	movl	-4(%rbp), %eax
	andl	$63, %eax
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE1348:
	.size	_ZN6NAHeap7bit2idxEj, .-_ZN6NAHeap7bit2idxEj
	.section	.text._ZN6NAHeap16computeTreeIndexEm,"axG",@progbits,_ZN6NAHeap16computeTreeIndexEm,comdat
	.weak	_ZN6NAHeap16computeTreeIndexEm
	.type	_ZN6NAHeap16computeTreeIndexEm, @function
_ZN6NAHeap16computeTreeIndexEm:
.LFB1349:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -40(%rbp)
	movq	-40(%rbp), %rax
	shrq	$8, %rax
	movq	%rax, -8(%rbp)
	cmpq	$0, -8(%rbp)
	jne	.L4
	movl	$0, %eax
	jmp	.L5
.L4:
	cmpq	$65535, -8(%rbp)
	jbe	.L6
	movl	$31, %eax
	jmp	.L5
.L6:
	movq	-8(%rbp), %rax
	movl	%eax, -20(%rbp)
	movl	-20(%rbp), %eax
	subl	$256, %eax
	shrl	$16, %eax
	andl	$8, %eax
	movl	%eax, -16(%rbp)
	movl	-16(%rbp), %eax
	movl	%eax, %ecx
	sall	%cl, -20(%rbp)
	movl	-20(%rbp), %eax
	subl	$4096, %eax
	shrl	$16, %eax
	andl	$4, %eax
	movl	%eax, -12(%rbp)
	movl	-12(%rbp), %eax
	addl	%eax, -16(%rbp)
	movl	-12(%rbp), %eax
	movl	%eax, %ecx
	sall	%cl, -20(%rbp)
	movl	-20(%rbp), %eax
	subl	$16384, %eax
	shrl	$16, %eax
	andl	$2, %eax
	movl	%eax, -12(%rbp)
	movl	-12(%rbp), %eax
	addl	%eax, -16(%rbp)
	movl	-12(%rbp), %eax
	movl	%eax, %ecx
	sall	%cl, -20(%rbp)
	movl	-20(%rbp), %eax
	shrl	$15, %eax
	subl	-16(%rbp), %eax
	addl	$14, %eax
	movl	%eax, -12(%rbp)
	movl	-12(%rbp), %eax
	leal	(%rax,%rax), %esi
	movl	-12(%rbp), %eax
	addl	$7, %eax
	movq	-40(%rbp), %rdx
	movl	%eax, %ecx
	shrq	%cl, %rdx
	movq	%rdx, %rax
	andl	$1, %eax
	addl	%esi, %eax
.L5:
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE1349:
	.size	_ZN6NAHeap16computeTreeIndexEm, .-_ZN6NAHeap16computeTreeIndexEm
	.section	.rodata
.LC0:
	.string	"%u,%u\n"
	.text
	.globl	main
	.type	main, @function
main:
.LFB1350:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	pushq	%rbx
	subq	$24, %rsp
	.cfi_offset 3, -24
	movl	$24, -20(%rbp)
	movl	-20(%rbp), %eax
	sall	$8, %eax
	movl	%eax, %eax
	movq	%rax, %rdi
	call	_ZN6NAHeap16computeTreeIndexEm
	movl	%eax, %ebx
	movl	-20(%rbp), %eax
	movl	%eax, %edi
	call	_ZN6NAHeap7bit2idxEj
	movl	%ebx, %edx
	movl	%eax, %esi
	leaq	.LC0(%rip), %rdi
	movl	$0, %eax
	call	printf@PLT
	movl	$0, %eax
	addq	$24, %rsp
	popq	%rbx
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE1350:
	.size	main, .-main
	.ident	"GCC: (Ubuntu 7.4.0-1ubuntu1~18.04.1) 7.4.0"
	.section	.note.GNU-stack,"",@progbits

	.file	"bogosort.c"
	.intel_syntax noprefix
	.text
	.section	.text.unlikely,"ax",@progbits
	.globl	avx512_bogosort
	.type	avx512_bogosort, @function
avx512_bogosort:
.LFB5597:
	.cfi_startproc
	endbr64
	push	rax
	.cfi_def_cfa_offset 16
	pop	rax
	.cfi_def_cfa_offset 8
	push	rax
	.cfi_def_cfa_offset 16
	call	abort@PLT
	.cfi_endproc
.LFE5597:
	.size	avx512_bogosort, .-avx512_bogosort
	.text
	.p2align 4
	.globl	set_taskset_enabled
	.type	set_taskset_enabled, @function
set_taskset_enabled:
.LFB5581:
	.cfi_startproc
	endbr64
	mov	DWORD PTR taskset_enabled[rip], edi
	ret
	.cfi_endproc
.LFE5581:
	.size	set_taskset_enabled, .-set_taskset_enabled
	.section	.rodata.str1.1,"aMS",@progbits,1
.LC0:
	.string	"yes"
.LC1:
	.string	"no"
.LC2:
	.string	"Taskset is enabled: %s\n"
	.text
	.p2align 4
	.globl	summarize_ts_enabled
	.type	summarize_ts_enabled, @function
summarize_ts_enabled:
.LFB5582:
	.cfi_startproc
	endbr64
	mov	eax, DWORD PTR taskset_enabled[rip]
	lea	rdx, .LC0[rip]
	test	eax, eax
	lea	rax, .LC1[rip]
	cmove	rdx, rax
	lea	rsi, .LC2[rip]
	mov	edi, 1
	xor	eax, eax
	jmp	__printf_chk@PLT
	.cfi_endproc
.LFE5582:
	.size	summarize_ts_enabled, .-summarize_ts_enabled
	.p2align 4
	.globl	sum_total_iters
	.type	sum_total_iters, @function
sum_total_iters:
.LFB5583:
	.cfi_startproc
	endbr64
	lea	rax, total_iters[rip]
	lea	rdx, 64[rax]
	xor	r8d, r8d
	.p2align 4,,10
	.p2align 3
.L9:
	add	r8, QWORD PTR [rax]
	add	rax, 8
	cmp	rax, rdx
	jne	.L9
	mov	rax, r8
	ret
	.cfi_endproc
.LFE5583:
	.size	sum_total_iters, .-sum_total_iters
	.p2align 4
	.globl	clear_total_iters
	.type	clear_total_iters, @function
clear_total_iters:
.LFB5584:
	.cfi_startproc
	endbr64
	vpxor	xmm0, xmm0, xmm0
	lea	rsi, last_cleared[rip]
	mov	edi, 1
	vmovaps	XMMWORD PTR total_iters[rip], xmm0
	vmovaps	XMMWORD PTR total_iters[rip+16], xmm0
	vmovaps	XMMWORD PTR total_iters[rip+32], xmm0
	vmovaps	XMMWORD PTR total_iters[rip+48], xmm0
	vmovaps	XMMWORD PTR measured_rdtsc[rip], xmm0
	vmovaps	XMMWORD PTR measured_rdtsc[rip+16], xmm0
	vmovaps	XMMWORD PTR measured_rdtsc[rip+32], xmm0
	vmovaps	XMMWORD PTR measured_rdtsc[rip+48], xmm0
	jmp	clock_gettime@PLT
	.cfi_endproc
.LFE5584:
	.size	clear_total_iters, .-clear_total_iters
	.section	.rodata.str1.1
.LC3:
	.string	"Thread %i iters: %llu\n"
.LC4:
	.string	"Thread %i rdtsc: %llu\n"
.LC5:
	.string	"\nTotal iters: %llu\n"
.LC6:
	.string	"\nTotal rdtsc: %llu\n"
.LC8:
	.string	"One iter every ns: %f\n"
	.text
	.p2align 4
	.globl	summarize_total_iters
	.type	summarize_total_iters, @function
summarize_total_iters:
.LFB5585:
	.cfi_startproc
	endbr64
	push	r15
	.cfi_def_cfa_offset 16
	.cfi_offset 15, -16
	xor	r15d, r15d
	push	r14
	.cfi_def_cfa_offset 24
	.cfi_offset 14, -24
	xor	r14d, r14d
	push	r13
	.cfi_def_cfa_offset 32
	.cfi_offset 13, -32
	lea	r13, total_iters[rip]
	push	r12
	.cfi_def_cfa_offset 40
	.cfi_offset 12, -40
	push	rbp
	.cfi_def_cfa_offset 48
	.cfi_offset 6, -48
	lea	rbp, measured_rdtsc[rip]
	push	rbx
	.cfi_def_cfa_offset 56
	.cfi_offset 3, -56
	xor	ebx, ebx
	sub	rsp, 56
	.cfi_def_cfa_offset 112
	mov	rax, QWORD PTR fs:40
	mov	QWORD PTR 40[rsp], rax
	xor	eax, eax
	jmp	.L15
	.p2align 4,,10
	.p2align 3
.L14:
	inc	rbx
	cmp	rbx, 8
	je	.L28
.L15:
	mov	rcx, QWORD PTR 0[r13+rbx*8]
	mov	r12, QWORD PTR 0[rbp+rbx*8]
	mov	r9d, ebx
	add	r15, rcx
	add	r14, r12
	test	rcx, rcx
	jne	.L29
.L13:
	test	r12, r12
	je	.L14
	mov	rcx, r12
	mov	edx, r9d
	lea	rsi, .LC4[rip]
	mov	edi, 1
	xor	eax, eax
	inc	rbx
	call	__printf_chk@PLT
	cmp	rbx, 8
	jne	.L15
.L28:
	mov	rdx, r15
	lea	rsi, .LC5[rip]
	mov	edi, 1
	xor	eax, eax
	call	__printf_chk@PLT
	mov	rdx, r14
	lea	rsi, .LC6[rip]
	mov	edi, 1
	xor	eax, eax
	call	__printf_chk@PLT
	lea	rsi, 16[rsp]
	mov	edi, 1
	call	clock_gettime@PLT
	mov	rax, QWORD PTR 16[rsp]
	vxorps	xmm2, xmm2, xmm2
	sub	rax, QWORD PTR last_cleared[rip]
	vcvtsi2sd	xmm1, xmm2, rax
	mov	rax, QWORD PTR 24[rsp]
	vmovsd	xmm4, QWORD PTR .LC7[rip]
	sub	rax, QWORD PTR last_cleared[rip+8]
	vmovapd	xmm3, xmm1
	vcvtsi2sd	xmm1, xmm2, rax
	vdivsd	xmm0, xmm1, xmm4
	vaddsd	xmm1, xmm0, xmm3
	test	r15, r15
	js	.L16
	vcvtsi2sd	xmm0, xmm2, r15
.L17:
	vdivsd	xmm0, xmm1, xmm0
	lea	rsi, .LC8[rip]
	mov	edi, 1
	mov	eax, 1
	vmulsd	xmm0, xmm0, xmm4
	call	__printf_chk@PLT
	mov	rax, QWORD PTR 40[rsp]
	xor	rax, QWORD PTR fs:40
	jne	.L30
	add	rsp, 56
	.cfi_remember_state
	.cfi_def_cfa_offset 56
	pop	rbx
	.cfi_def_cfa_offset 48
	pop	rbp
	.cfi_def_cfa_offset 40
	pop	r12
	.cfi_def_cfa_offset 32
	pop	r13
	.cfi_def_cfa_offset 24
	pop	r14
	.cfi_def_cfa_offset 16
	pop	r15
	.cfi_def_cfa_offset 8
	ret
	.p2align 4,,10
	.p2align 3
.L29:
	.cfi_restore_state
	mov	edx, ebx
	lea	rsi, .LC3[rip]
	mov	edi, 1
	xor	eax, eax
	mov	DWORD PTR 12[rsp], ebx
	call	__printf_chk@PLT
	mov	r9d, DWORD PTR 12[rsp]
	jmp	.L13
	.p2align 4,,10
	.p2align 3
.L16:
	mov	rax, r15
	shr	rax
	and	r15d, 1
	or	rax, r15
	vcvtsi2sd	xmm0, xmm2, rax
	vaddsd	xmm0, xmm0, xmm0
	jmp	.L17
.L30:
	call	__stack_chk_fail@PLT
	.cfi_endproc
.LFE5585:
	.size	summarize_total_iters, .-summarize_total_iters
	.p2align 4
	.globl	core_count
	.type	core_count, @function
core_count:
.LFB5586:
	.cfi_startproc
	endbr64
	sub	rsp, 8
	.cfi_def_cfa_offset 16
	mov	edi, 84
	call	sysconf@PLT
	add	rsp, 8
	.cfi_def_cfa_offset 8
	ret
	.cfi_endproc
.LFE5586:
	.size	core_count, .-core_count
	.section	.text.unlikely
.LCOLDB9:
	.text
.LHOTB9:
	.p2align 4
	.globl	taskset_thread_self
	.type	taskset_thread_self, @function
taskset_thread_self:
.LFB5587:
	.cfi_startproc
	endbr64
	push	r12
	.cfi_def_cfa_offset 16
	.cfi_offset 12, -16
	push	rbx
	.cfi_def_cfa_offset 24
	.cfi_offset 3, -24
	mov	ebx, edi
	mov	edi, 84
	sub	rsp, 152
	.cfi_def_cfa_offset 176
	mov	rax, QWORD PTR fs:40
	mov	QWORD PTR 136[rsp], rax
	xor	eax, eax
	call	sysconf@PLT
	cmp	ebx, eax
	jge	.L40
	mov	r12, rsp
	xor	edx, edx
	xor	eax, eax
.L35:
	mov	ecx, eax
	add	eax, 32
	mov	QWORD PTR [r12+rcx], rdx
	mov	QWORD PTR 8[r12+rcx], rdx
	mov	QWORD PTR 16[r12+rcx], rdx
	mov	QWORD PTR 24[r12+rcx], rdx
	cmp	eax, 128
	jb	.L35
	movsx	rax, ebx
	cmp	rax, 1023
	jbe	.L43
.L37:
	call	pthread_self@PLT
	mov	rdi, rax
	mov	rdx, r12
	mov	esi, 128
	call	pthread_setaffinity_np@PLT
	mov	rax, QWORD PTR 136[rsp]
	xor	rax, QWORD PTR fs:40
	jne	.L44
	add	rsp, 152
	.cfi_remember_state
	.cfi_def_cfa_offset 24
	pop	rbx
	.cfi_def_cfa_offset 16
	pop	r12
	.cfi_def_cfa_offset 8
	ret
	.p2align 4,,10
	.p2align 3
.L43:
	.cfi_restore_state
	shr	rax, 6
	mov	edi, 1
	shlx	rbx, rdi, rbx
	or	QWORD PTR [r12+rax*8], rbx
	jmp	.L37
.L44:
	call	__stack_chk_fail@PLT
	.cfi_endproc
	.section	.text.unlikely
	.cfi_startproc
	.type	taskset_thread_self.cold, @function
taskset_thread_self.cold:
.LFSB5587:
.L40:
	.cfi_def_cfa_offset 176
	.cfi_offset 3, -24
	.cfi_offset 12, -16
	mov	DWORD PTR complete[rip], 1
	call	abort@PLT
	.cfi_endproc
.LFE5587:
	.text
	.size	taskset_thread_self, .-taskset_thread_self
	.section	.text.unlikely
	.size	taskset_thread_self.cold, .-taskset_thread_self.cold
.LCOLDE9:
	.text
.LHOTE9:
	.section	.rodata.str1.1
.LC11:
	.string	"Thread %i found!\n"
	.text
	.p2align 4
	.globl	avx2_bogosort
	.type	avx2_bogosort, @function
avx2_bogosort:
.LFB5599:
	.cfi_startproc
	endbr64
	push	rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	mov	rbp, rsp
	.cfi_def_cfa_register 6
	push	r12
	.cfi_offset 12, -24
	xor	r12d, r12d
	push	rbx
	and	rsp, -32
	sub	rsp, 96
	.cfi_offset 3, -32
	test	rdi, rdi
	je	.L46
	movsx	r12, DWORD PTR [rdi]
.L46:
	lea	rdi, result_mutex[rip]
	call	pthread_mutex_lock@PLT
	mov	rbx, QWORD PTR SEED[rip]
	lea	rdi, result_mutex[rip]
	lea	rax, 1[rbx]
	mov	QWORD PTR SEED[rip], rax
	call	pthread_mutex_unlock@PLT
	mov	ecx, DWORD PTR taskset_enabled[rip]
	test	ecx, ecx
	jne	.L66
.L47:
	movabs	rsi, 6364136223846793005
	imul	rbx, rsi
	mov	rcx, QWORD PTR shuffles[rip]
	mov	edx, DWORD PTR complete[rip]
	lea	rax, 1[rbx]
	vmovdqa	ymm3, YMMWORD PTR result[rip]
	vmovdqa	ymm4, YMMWORD PTR result[rip+32]
	vmovdqa	ymm6, YMMWORD PTR 96[rcx]
	vmovdqa	ymm5, YMMWORD PTR 64[rcx]
	xor	ebx, ebx
	test	edx, edx
	jne	.L48
	vmovdqa	ymm7, YMMWORD PTR .LC10[rip]
	jmp	.L51
	.p2align 4,,10
	.p2align 3
.L49:
	mov	edx, DWORD PTR complete[rip]
	test	edx, edx
	jne	.L48
.L51:
	imul	rax, rsi
	vpermd	ymm0, ymm6, ymm3
	vpermd	ymm1, ymm5, ymm4
	vpunpckhdq	ymm2, ymm0, ymm1
	inc	rax
	vpunpckldq	ymm1, ymm1, ymm0
	mov	rdx, rax
	vpermd	ymm0, ymm7, ymm2
	and	edx, 65504
	vpcmpgtd	ymm0, ymm0, ymm2
	vmovdqa	ymm6, YMMWORD PTR [rcx+rdx]
	mov	rdx, rax
	shr	rdx, 53
	sal	rdx, 5
	inc	rbx
	vtestps	ymm0, ymm0
	vmovdqa	ymm5, YMMWORD PTR [rcx+rdx]
	vmovdqa	ymm3, ymm2
	vmovdqa	ymm4, ymm1
	jne	.L49
	vextracti128	xmm8, ymm2, 0x1
	vpermd	ymm0, ymm7, ymm1
	vpsrldq	xmm9, xmm8, 12
	vmovss	xmm8, xmm0, xmm9
	vinserti128	ymm0, ymm0, xmm8, 0x0
	vpcmpgtd	ymm0, ymm0, ymm1
	vtestps	ymm0, ymm0
	vmovdqa	YMMWORD PTR 64[rsp], ymm0
	jne	.L49
	lea	rdi, result_mutex[rip]
	vmovdqa	YMMWORD PTR [rsp], ymm1
	vmovdqa	YMMWORD PTR 32[rsp], ymm2
	vzeroupper
	call	pthread_mutex_lock@PLT
	mov	eax, DWORD PTR complete[rip]
	test	eax, eax
	jne	.L48
	vmovdqa	ymm2, YMMWORD PTR 32[rsp]
	vmovdqa	ymm1, YMMWORD PTR [rsp]
	mov	edx, DWORD PTR print_which_thread_found[rip]
	mov	DWORD PTR complete[rip], 1
	vmovdqa	YMMWORD PTR result[rip], ymm2
	vmovdqa	YMMWORD PTR result[rip+32], ymm1
	test	edx, edx
	jne	.L67
	vzeroupper
.L50:
	lea	rdi, result_mutex[rip]
	call	pthread_mutex_unlock@PLT
.L48:
	vmovdqa	ymm7, YMMWORD PTR 64[rsp]
	lea	rdi, result_mutex[rip]
	vmovdqa	YMMWORD PTR result[rip+64], ymm7
	vzeroupper
	call	pthread_mutex_lock@PLT
	lea	rax, total_iters[rip]
	lea	rdi, result_mutex[rip]
	add	QWORD PTR [rax+r12*8], rbx
	call	pthread_mutex_unlock@PLT
	lea	rsp, -16[rbp]
	pop	rbx
	pop	r12
	xor	eax, eax
	pop	rbp
	.cfi_remember_state
	.cfi_def_cfa 7, 8
	ret
	.p2align 4,,10
	.p2align 3
.L66:
	.cfi_restore_state
	mov	edi, r12d
	call	taskset_thread_self
	jmp	.L47
.L67:
	mov	edx, r12d
	lea	rsi, .LC11[rip]
	mov	edi, 1
	vzeroupper
	call	__printf_chk@PLT
	jmp	.L50
	.cfi_endproc
.LFE5599:
	.size	avx2_bogosort, .-avx2_bogosort
	.p2align 4
	.globl	rand_range
	.type	rand_range, @function
rand_range:
.LFB5588:
	.cfi_startproc
	endbr64
	movabs	rax, 6018048192831
	imul	rax, QWORD PTR r[rip]
	movsx	rdi, edi
	xor	edx, edx
	inc	rax
	mov	QWORD PTR r[rip], rax
	shr	rax, 9
	div	rdi
	mov	eax, edx
	ret
	.cfi_endproc
.LFE5588:
	.size	rand_range, .-rand_range
	.p2align 4
	.globl	is_sorted
	.type	is_sorted, @function
is_sorted:
.LFB5589:
	.cfi_startproc
	endbr64
	cmp	esi, 1
	jle	.L72
	lea	eax, -2[rsi]
	lea	rax, 4[rdi+rax*4]
	jmp	.L71
	.p2align 4,,10
	.p2align 3
.L76:
	add	rdi, 4
	cmp	rdi, rax
	je	.L72
.L71:
	mov	edx, DWORD PTR 4[rdi]
	cmp	DWORD PTR [rdi], edx
	jle	.L76
	xor	eax, eax
	ret
	.p2align 4,,10
	.p2align 3
.L72:
	mov	eax, 1
	ret
	.cfi_endproc
.LFE5589:
	.size	is_sorted, .-is_sorted
	.p2align 4
	.globl	shuffle
	.type	shuffle, @function
shuffle:
.LFB5590:
	.cfi_startproc
	endbr64
	lea	ecx, -1[rsi]
	test	ecx, ecx
	jle	.L81
	movsx	r9, esi
	movsx	rcx, ecx
	dec	r9
	lea	eax, -2[rsi]
	mov	r8, QWORD PTR r[rip]
	inc	rcx
	sub	r9, rax
	movabs	r10, 6018048192831
	.p2align 4,,10
	.p2align 3
.L79:
	imul	r8, r10
	xor	edx, edx
	mov	esi, DWORD PTR -4[rdi+rcx*4]
	inc	r8
	mov	rax, r8
	shr	rax, 9
	div	rcx
	lea	rax, [rdi+rdx*4]
	mov	edx, DWORD PTR [rax]
	mov	DWORD PTR -4[rdi+rcx*4], edx
	dec	rcx
	mov	DWORD PTR [rax], esi
	cmp	r9, rcx
	jne	.L79
	mov	QWORD PTR r[rip], r8
.L81:
	ret
	.cfi_endproc
.LFE5590:
	.size	shuffle, .-shuffle
	.section	.rodata.str1.1
.LC12:
	.string	"%i "
	.text
	.p2align 4
	.globl	print_arr
	.type	print_arr, @function
print_arr:
.LFB5591:
	.cfi_startproc
	endbr64
	test	esi, esi
	jle	.L87
	push	r12
	.cfi_def_cfa_offset 16
	.cfi_offset 12, -16
	lea	eax, -1[rsi]
	lea	r12, 4[rdi+rax*4]
	push	rbp
	.cfi_def_cfa_offset 24
	.cfi_offset 6, -24
	lea	rbp, .LC12[rip]
	push	rbx
	.cfi_def_cfa_offset 32
	.cfi_offset 3, -32
	mov	rbx, rdi
	.p2align 4,,10
	.p2align 3
.L84:
	mov	edx, DWORD PTR [rbx]
	mov	rsi, rbp
	mov	edi, 1
	xor	eax, eax
	add	rbx, 4
	call	__printf_chk@PLT
	cmp	rbx, r12
	jne	.L84
	pop	rbx
	.cfi_restore 3
	.cfi_def_cfa_offset 24
	pop	rbp
	.cfi_restore 6
	.cfi_def_cfa_offset 16
	mov	edi, 10
	pop	r12
	.cfi_restore 12
	.cfi_def_cfa_offset 8
	jmp	putchar@PLT
	.p2align 4,,10
	.p2align 3
.L87:
	mov	edi, 10
	jmp	putchar@PLT
	.cfi_endproc
.LFE5591:
	.size	print_arr, .-print_arr
	.p2align 4
	.globl	standard_bogosort
	.type	standard_bogosort, @function
standard_bogosort:
.LFB5592:
	.cfi_startproc
	endbr64
	push	rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	lea	eax, -2[rsi]
	lea	r11, 4[rdi+rax*4]
	push	rbx
	.cfi_def_cfa_offset 24
	.cfi_offset 3, -24
	mov	ebx, esi
	xor	ebp, ebp
	cmp	ebx, 1
	jle	.L91
	.p2align 4,,10
	.p2align 3
.L100:
	mov	rax, rdi
	jmp	.L93
	.p2align 4,,10
	.p2align 3
.L99:
	add	rax, 4
	cmp	rax, r11
	je	.L91
.L93:
	mov	edx, DWORD PTR 4[rax]
	cmp	DWORD PTR [rax], edx
	jle	.L99
	mov	esi, ebx
	call	shuffle
	inc	rbp
	cmp	ebx, 1
	jg	.L100
.L91:
	pop	rbx
	.cfi_def_cfa_offset 16
	add	QWORD PTR total_iters[rip], rbp
	pop	rbp
	.cfi_def_cfa_offset 8
	ret
	.cfi_endproc
.LFE5592:
	.size	standard_bogosort, .-standard_bogosort
	.p2align 4
	.globl	log_mm256
	.type	log_mm256, @function
log_mm256:
.LFB5593:
	.cfi_startproc
	endbr64
	push	rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	mov	rbp, rsp
	.cfi_def_cfa_register 6
	push	r13
	push	r12
	.cfi_offset 13, -24
	.cfi_offset 12, -32
	lea	r12, .LC12[rip]
	push	rbx
	and	rsp, -32
	sub	rsp, 64
	.cfi_offset 3, -40
	mov	rax, QWORD PTR fs:40
	mov	QWORD PTR 56[rsp], rax
	xor	eax, eax
	lea	rbx, 16[rsp]
	lea	r13, 48[rsp]
	vmovdqu	YMMWORD PTR 16[rsp], ymm0
	vzeroupper
	.p2align 4,,10
	.p2align 3
.L102:
	mov	edx, DWORD PTR [rbx]
	mov	rsi, r12
	mov	edi, 1
	xor	eax, eax
	add	rbx, 4
	call	__printf_chk@PLT
	cmp	rbx, r13
	jne	.L102
	mov	rax, QWORD PTR 56[rsp]
	xor	rax, QWORD PTR fs:40
	jne	.L107
	lea	rsp, -24[rbp]
	pop	rbx
	pop	r12
	pop	r13
	mov	edi, 10
	pop	rbp
	.cfi_remember_state
	.cfi_def_cfa 7, 8
	jmp	putchar@PLT
.L107:
	.cfi_restore_state
	call	__stack_chk_fail@PLT
	.cfi_endproc
.LFE5593:
	.size	log_mm256, .-log_mm256
	.p2align 4
	.globl	fill_shuffles
	.type	fill_shuffles, @function
fill_shuffles:
.LFB5594:
	.cfi_startproc
	endbr64
	push	rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	mov	edi, 64
	mov	esi, 65536
	mov	rbp, rsp
	.cfi_def_cfa_register 6
	push	r15
	push	r14
	push	r13
	.cfi_offset 15, -24
	.cfi_offset 14, -32
	.cfi_offset 13, -40
	xor	r13d, r13d
	push	r12
	push	rbx
	and	rsp, -32
	sub	rsp, 64
	.cfi_offset 12, -48
	.cfi_offset 3, -56
	mov	rax, QWORD PTR fs:40
	mov	QWORD PTR 56[rsp], rax
	xor	eax, eax
	call	aligned_alloc@PLT
	mov	QWORD PTR shuffles[rip], rax
	mov	rbx, rax
	movabs	rax, 4294967296
	mov	QWORD PTR 16[rsp], rax
	movabs	rax, 12884901890
	mov	QWORD PTR 24[rsp], rax
	movabs	rax, 21474836484
	mov	QWORD PTR 32[rsp], rax
	movabs	rax, 30064771078
	mov	QWORD PTR 40[rsp], rax
	lea	r11, shifts[rip]
	lea	rdi, 16[rsp]
	lea	r14, 48[rsp]
.L109:
	mov	r12, rbx
	xor	r15d, r15d
	.p2align 4,,10
	.p2align 3
.L111:
	mov	esi, 8
	call	shuffle
	mov	rdx, rdi
	mov	rax, r12
	.p2align 4,,10
	.p2align 3
.L110:
	shlx	ecx, DWORD PTR [rdx], r13d
	add	rdx, 4
	or	DWORD PTR [rax], ecx
	add	rax, 4
	cmp	rdx, r14
	jne	.L110
	add	r15d, 8
	add	r12, 32
	cmp	r15d, 16384
	jne	.L111
	vmovd	xmm0, r13d
	vpbroadcastd	ymm0, xmm0
	vmovdqa	YMMWORD PTR [r11], ymm0
	lea	rax, shifts[rip+256]
	add	r11, 32
	add	r13d, 3
	cmp	rax, r11
	je	.L108
	vzeroupper
	jmp	.L109
.L108:
	mov	rax, QWORD PTR 56[rsp]
	xor	rax, QWORD PTR fs:40
	jne	.L118
	vzeroupper
	lea	rsp, -40[rbp]
	pop	rbx
	pop	r12
	pop	r13
	pop	r14
	pop	r15
	pop	rbp
	.cfi_remember_state
	.cfi_def_cfa 7, 8
	ret
.L118:
	.cfi_restore_state
	vzeroupper
	call	__stack_chk_fail@PLT
	.cfi_endproc
.LFE5594:
	.size	fill_shuffles, .-fill_shuffles
	.p2align 4
	.globl	get_8x32_shuffle
	.type	get_8x32_shuffle, @function
get_8x32_shuffle:
.LFB5595:
	.cfi_startproc
	endbr64
	movsx	rdi, edi
	sal	rdi, 5
	add	rdi, QWORD PTR shuffles[rip]
	vmovdqa	ymm0, YMMWORD PTR [rdi]
	ret
	.cfi_endproc
.LFE5595:
	.size	get_8x32_shuffle, .-get_8x32_shuffle
	.p2align 4
	.globl	get_shuffle_shift
	.type	get_shuffle_shift, @function
get_shuffle_shift:
.LFB5596:
	.cfi_startproc
	endbr64
	movsx	rdi, edi
	sal	rdi, 5
	lea	rax, shifts[rip]
	vmovdqa	ymm0, YMMWORD PTR [rax+rdi]
	ret
	.cfi_endproc
.LFE5596:
	.size	get_shuffle_shift, .-get_shuffle_shift
	.p2align 4
	.globl	next_r
	.type	next_r, @function
next_r:
.LFB5598:
	.cfi_startproc
	endbr64
	movabs	rax, 6364136223846793005
	imul	rdi, rax
	lea	rax, 1[rdi]
	ret
	.cfi_endproc
.LFE5598:
	.size	next_r, .-next_r
	.p2align 4
	.globl	fill_nonzero_elems
	.type	fill_nonzero_elems, @function
fill_nonzero_elems:
.LFB5600:
	.cfi_startproc
	endbr64
	test	edi, edi
	jle	.L123
	lea	edx, -1[rdi]
	add	rdx, 2
	mov	eax, 1
	lea	rcx, result[rip-4]
	.p2align 4,,10
	.p2align 3
.L124:
	mov	DWORD PTR [rcx+rax*4], eax
	inc	rax
	cmp	rax, rdx
	jne	.L124
	cmp	edi, 15
	jg	.L129
.L123:
	mov	edx, 15
	movsx	rcx, edi
	lea	rax, result[rip]
	sub	edx, edi
	add	rdx, rcx
	lea	rax, [rax+rcx*4]
	lea	rcx, result[rip+4]
	lea	rdx, [rcx+rdx*4]
	.p2align 4,,10
	.p2align 3
.L126:
	mov	DWORD PTR [rax], 0
	add	rax, 4
	cmp	rax, rdx
	jne	.L126
.L129:
	ret
	.cfi_endproc
.LFE5600:
	.size	fill_nonzero_elems, .-fill_nonzero_elems
	.p2align 4
	.globl	time_start
	.type	time_start, @function
time_start:
.LFB5601:
	.cfi_startproc
	endbr64
	movsx	rsi, DWORD PTR timespec_i[rip]
	push	rbx
	.cfi_def_cfa_offset 16
	.cfi_offset 3, -16
	lea	rax, start_a[rip]
	mov	rbx, rsi
	sal	rsi, 4
	inc	ebx
	add	rsi, rax
	mov	edi, 1
	call	clock_gettime@PLT
	mov	DWORD PTR timespec_i[rip], ebx
	pop	rbx
	.cfi_def_cfa_offset 8
	ret
	.cfi_endproc
.LFE5601:
	.size	time_start, .-time_start
	.p2align 4
	.globl	grab_elapsed
	.type	grab_elapsed, @function
grab_elapsed:
.LFB5602:
	.cfi_startproc
	endbr64
	sub	rsp, 40
	.cfi_def_cfa_offset 48
	mov	rsi, rsp
	mov	edi, 1
	mov	rax, QWORD PTR fs:40
	mov	QWORD PTR 24[rsp], rax
	xor	eax, eax
	call	clock_gettime@PLT
	mov	eax, DWORD PTR timespec_i[rip]
	vxorps	xmm2, xmm2, xmm2
	lea	edx, -1[rax]
	movsx	rdx, edx
	sal	rdx, 4
	lea	rax, start_a[rip]
	add	rdx, rax
	mov	rax, QWORD PTR 8[rsp]
	sub	rax, QWORD PTR 8[rdx]
	vcvtsi2sd	xmm0, xmm2, rax
	mov	rax, QWORD PTR [rsp]
	sub	rax, QWORD PTR [rdx]
	vdivsd	xmm1, xmm0, QWORD PTR .LC7[rip]
	vcvtsi2sd	xmm0, xmm2, rax
	vaddsd	xmm0, xmm1, xmm0
	vmovsd	QWORD PTR elapsed[rip], xmm0
	mov	rax, QWORD PTR 24[rsp]
	xor	rax, QWORD PTR fs:40
	jne	.L136
	add	rsp, 40
	.cfi_remember_state
	.cfi_def_cfa_offset 8
	ret
.L136:
	.cfi_restore_state
	call	__stack_chk_fail@PLT
	.cfi_endproc
.LFE5602:
	.size	grab_elapsed, .-grab_elapsed
	.section	.rodata.str1.8,"aMS",@progbits,1
	.align 8
.LC13:
	.string	"Timer %s finished: %f seconds\n"
	.text
	.p2align 4
	.globl	time_end
	.type	time_end, @function
time_end:
.LFB5603:
	.cfi_startproc
	endbr64
	test	rdi, rdi
	je	.L143
	push	r12
	.cfi_def_cfa_offset 16
	.cfi_offset 12, -16
	xor	eax, eax
	mov	r12, rdi
	call	grab_elapsed
	mov	rdx, r12
	lea	rsi, .LC13[rip]
	mov	edi, 1
	mov	eax, 1
	call	__printf_chk@PLT
	dec	DWORD PTR timespec_i[rip]
	pop	r12
	.cfi_def_cfa_offset 8
	ret
	.p2align 4,,10
	.p2align 3
.L143:
	.cfi_restore 12
	dec	DWORD PTR timespec_i[rip]
	ret
	.cfi_endproc
.LFE5603:
	.size	time_end, .-time_end
	.section	.text.unlikely
.LCOLDB14:
	.text
.LHOTB14:
	.p2align 4
	.globl	run_accel_bogosort
	.type	run_accel_bogosort, @function
run_accel_bogosort:
.LFB5604:
	.cfi_startproc
	endbr64
	push	r15
	.cfi_def_cfa_offset 16
	.cfi_offset 15, -16
	push	r14
	.cfi_def_cfa_offset 24
	.cfi_offset 14, -24
	push	r13
	.cfi_def_cfa_offset 32
	.cfi_offset 13, -32
	push	r12
	.cfi_def_cfa_offset 40
	.cfi_offset 12, -40
	push	rbp
	.cfi_def_cfa_offset 48
	.cfi_offset 6, -48
	push	rbx
	.cfi_def_cfa_offset 56
	.cfi_offset 3, -56
	sub	rsp, 136
	.cfi_def_cfa_offset 192
	mov	DWORD PTR 12[rsp], esi
	mov	rax, QWORD PTR fs:40
	mov	QWORD PTR 120[rsp], rax
	xor	eax, eax
	mov	DWORD PTR complete[rip], 0
	cmp	edi, 1
	jle	.L147
	test	esi, esi
	lea	r14, avx512_bogosort[rip]
	lea	rax, avx2_bogosort[rip]
	lea	rbp, 56[rsp]
	cmove	r14, rax
	mov	r13d, edi
	lea	r12, 20[rsp]
	mov	r15, rbp
	mov	ebx, 1
	.p2align 4,,10
	.p2align 3
.L148:
	mov	rcx, r12
	mov	rdx, r14
	mov	rdi, r15
	xor	esi, esi
	mov	DWORD PTR [r12], ebx
	call	pthread_create@PLT
	mov	edx, ebx
	inc	ebx
	add	r12, 4
	add	r15, 8
	cmp	r13d, ebx
	jne	.L148
	mov	ecx, DWORD PTR 12[rsp]
	test	ecx, ecx
	jne	.L163
	xor	edi, edi
	mov	DWORD PTR 12[rsp], edx
	call	avx2_bogosort
	mov	edx, DWORD PTR 12[rsp]
	lea	eax, -1[rdx]
	lea	rbx, 64[rsp+rax*8]
	.p2align 4,,10
	.p2align 3
.L150:
	mov	rdi, QWORD PTR 0[rbp]
	xor	esi, esi
	add	rbp, 8
	call	pthread_join@PLT
	cmp	rbp, rbx
	jne	.L150
.L146:
	mov	rax, QWORD PTR 120[rsp]
	xor	rax, QWORD PTR fs:40
	jne	.L164
	add	rsp, 136
	.cfi_remember_state
	.cfi_def_cfa_offset 56
	pop	rbx
	.cfi_def_cfa_offset 48
	pop	rbp
	.cfi_def_cfa_offset 40
	pop	r12
	.cfi_def_cfa_offset 32
	pop	r13
	.cfi_def_cfa_offset 24
	pop	r14
	.cfi_def_cfa_offset 16
	pop	r15
	.cfi_def_cfa_offset 8
	ret
	.p2align 4,,10
	.p2align 3
.L147:
	.cfi_restore_state
	mov	eax, DWORD PTR 12[rsp]
	test	eax, eax
	jne	.L165
	xor	edi, edi
	call	avx2_bogosort
	jmp	.L146
.L164:
	call	__stack_chk_fail@PLT
.L165:
	jmp	.L151
.L163:
	jmp	.L151
	.cfi_endproc
	.section	.text.unlikely
	.cfi_startproc
	.type	run_accel_bogosort.cold, @function
run_accel_bogosort.cold:
.LFSB5604:
.L151:
	.cfi_def_cfa_offset 192
	.cfi_offset 3, -56
	.cfi_offset 6, -48
	.cfi_offset 12, -40
	.cfi_offset 13, -32
	.cfi_offset 14, -24
	.cfi_offset 15, -16
	call	abort@PLT
	.cfi_endproc
.LFE5604:
	.text
	.size	run_accel_bogosort, .-run_accel_bogosort
	.section	.text.unlikely
	.size	run_accel_bogosort.cold, .-run_accel_bogosort.cold
.LCOLDE14:
	.text
.LHOTE14:
	.section	.rodata.str1.8
	.align 8
.LC15:
	.string	"\n\nNonzero elems,Iters,Time in s,One iter every ns,Average ms per case"
	.text
	.p2align 4
	.globl	print_header
	.type	print_header, @function
print_header:
.LFB5605:
	.cfi_startproc
	endbr64
	lea	rdi, .LC15[rip]
	jmp	puts@PLT
	.cfi_endproc
.LFE5605:
	.size	print_header, .-print_header
	.section	.rodata.str1.1
.LC17:
	.string	"%i,%i,%f,%f,%f\n"
.LC18:
	.string	"bogosort_nonzero"
	.text
	.p2align 4
	.globl	run_bogosort_nonzero
	.type	run_bogosort_nonzero, @function
run_bogosort_nonzero:
.LFB5606:
	.cfi_startproc
	endbr64
	push	r15
	.cfi_def_cfa_offset 16
	.cfi_offset 15, -16
	lea	rax, start_a[rip]
	mov	r15d, edx
	push	r14
	.cfi_def_cfa_offset 24
	.cfi_offset 14, -24
	push	r13
	.cfi_def_cfa_offset 32
	.cfi_offset 13, -32
	push	r12
	.cfi_def_cfa_offset 40
	.cfi_offset 12, -40
	push	rbp
	.cfi_def_cfa_offset 48
	.cfi_offset 6, -48
	mov	ebp, edi
	mov	edi, 1
	push	rbx
	.cfi_def_cfa_offset 56
	.cfi_offset 3, -56
	mov	ebx, esi
	sub	rsp, 56
	.cfi_def_cfa_offset 112
	movsx	rsi, DWORD PTR timespec_i[rip]
	mov	DWORD PTR 36[rsp], edx
	mov	r12, rsi
	sal	rsi, 4
	add	rsi, rax
	call	clock_gettime@PLT
	lea	edx, 1[r12]
	mov	DWORD PTR timespec_i[rip], edx
	cmp	ebx, r15d
	jge	.L168
	movsx	rax, ebx
	mov	rdi, rax
	mov	QWORD PTR 16[rsp], rax
	lea	rax, result[rip]
	lea	rax, [rax+rdi*4]
	mov	QWORD PTR [rsp], rax
	vxorpd	xmm6, xmm6, xmm6
	mov	eax, 15
	sub	eax, ebx
	vcvtsi2sd	xmm0, xmm6, ebp
	mov	DWORD PTR 12[rsp], eax
	lea	eax, -1[rbx]
	mov	DWORD PTR 32[rsp], eax
	vmovsd	QWORD PTR 40[rsp], xmm0
	lea	r15, result[rip-4]
	.p2align 4,,10
	.p2align 3
.L180:
	movsx	rsi, edx
	sal	rsi, 4
	lea	rax, start_a[rip]
	add	rsi, rax
	mov	edi, 1
	mov	DWORD PTR 24[rsp], edx
	call	clock_gettime@PLT
	mov	edx, DWORD PTR 24[rsp]
	xor	eax, eax
	inc	edx
	mov	DWORD PTR timespec_i[rip], edx
	call	clear_total_iters
	test	ebp, ebp
	jle	.L170
	mov	eax, DWORD PTR 12[rsp]
	mov	edx, DWORD PTR 32[rsp]
	add	rax, QWORD PTR 16[rsp]
	lea	rcx, result[rip+4]
	xor	r14d, r14d
	lea	r13, 2[rdx]
	lea	r12, [rcx+rax*4]
	mov	eax, ebx
	mov	ebx, r14d
	mov	r14, r13
	mov	r13d, eax
	.p2align 4,,10
	.p2align 3
.L173:
	mov	eax, 1
	test	r13d, r13d
	jle	.L172
	.p2align 4,,10
	.p2align 3
.L171:
	mov	DWORD PTR [r15+rax*4], eax
	inc	rax
	cmp	rax, r14
	jne	.L171
	cmp	r13d, 15
	jg	.L175
.L172:
	mov	rax, QWORD PTR [rsp]
	.p2align 4,,10
	.p2align 3
.L174:
	mov	DWORD PTR [rax], 0
	add	rax, 4
	cmp	rax, r12
	jne	.L174
.L175:
	mov	esi, 16
	lea	rdi, result[rip]
	call	shuffle
	inc	ebx
	mov	esi, 16
	lea	rdi, result[rip]
	call	standard_bogosort
	cmp	ebp, ebx
	jne	.L173
	mov	ebx, r13d
.L170:
	xor	eax, eax
	call	grab_elapsed
	lea	rdi, .LC15[rip]
	vmovsd	QWORD PTR 24[rsp], xmm0
	call	puts@PLT
	vmovsd	xmm0, QWORD PTR 24[rsp]
	lea	rax, total_iters[rip]
	vdivsd	xmm2, xmm0, QWORD PTR 40[rsp]
	vmulsd	xmm2, xmm2, QWORD PTR .LC16[rip]
	xor	edx, edx
	.p2align 4,,10
	.p2align 3
.L177:
	add	rdx, QWORD PTR [rax]
	lea	rcx, total_iters[rip+64]
	add	rax, 8
	cmp	rcx, rax
	jne	.L177
	test	rdx, rdx
	js	.L178
	vxorpd	xmm4, xmm4, xmm4
	vcvtsi2sd	xmm1, xmm4, rdx
.L179:
	vdivsd	xmm1, xmm0, xmm1
	mov	edx, ebx
	mov	ecx, ebp
	lea	rsi, .LC17[rip]
	mov	edi, 1
	mov	eax, 3
	inc	ebx
	vmulsd	xmm1, xmm1, QWORD PTR .LC7[rip]
	call	__printf_chk@PLT
	xor	eax, eax
	dec	DWORD PTR timespec_i[rip]
	call	summarize_total_iters
	add	QWORD PTR [rsp], 4
	dec	DWORD PTR 12[rsp]
	inc	QWORD PTR 16[rsp]
	inc	DWORD PTR 32[rsp]
	cmp	DWORD PTR 36[rsp], ebx
	je	.L168
	mov	edx, DWORD PTR timespec_i[rip]
	jmp	.L180
.L178:
	mov	rax, rdx
	shr	rax
	and	edx, 1
	or	rax, rdx
	vxorpd	xmm5, xmm5, xmm5
	vcvtsi2sd	xmm1, xmm5, rax
	vaddsd	xmm1, xmm1, xmm1
	jmp	.L179
.L168:
	add	rsp, 56
	.cfi_def_cfa_offset 56
	pop	rbx
	.cfi_def_cfa_offset 48
	pop	rbp
	.cfi_def_cfa_offset 40
	pop	r12
	.cfi_def_cfa_offset 32
	pop	r13
	.cfi_def_cfa_offset 24
	pop	r14
	.cfi_def_cfa_offset 16
	lea	rdi, .LC18[rip]
	pop	r15
	.cfi_def_cfa_offset 8
	jmp	time_end
	.cfi_endproc
.LFE5606:
	.size	run_bogosort_nonzero, .-run_bogosort_nonzero
	.section	.rodata.str1.1
.LC19:
	.string	"bogosort"
	.text
	.p2align 4
	.globl	run_bogosort
	.type	run_bogosort, @function
run_bogosort:
.LFB5607:
	.cfi_startproc
	endbr64
	push	r15
	.cfi_def_cfa_offset 16
	.cfi_offset 15, -16
	lea	rax, start_a[rip]
	mov	r15d, edx
	push	r14
	.cfi_def_cfa_offset 24
	.cfi_offset 14, -24
	mov	r14d, esi
	push	r13
	.cfi_def_cfa_offset 32
	.cfi_offset 13, -32
	push	r12
	.cfi_def_cfa_offset 40
	.cfi_offset 12, -40
	push	rbp
	.cfi_def_cfa_offset 48
	.cfi_offset 6, -48
	push	rbx
	.cfi_def_cfa_offset 56
	.cfi_offset 3, -56
	mov	ebx, edi
	mov	edi, 1
	sub	rsp, 56
	.cfi_def_cfa_offset 112
	movsx	rsi, DWORD PTR timespec_i[rip]
	mov	DWORD PTR 36[rsp], edx
	mov	rbp, rsi
	sal	rsi, 4
	add	rsi, rax
	call	clock_gettime@PLT
	lea	edx, 1[rbp]
	mov	DWORD PTR timespec_i[rip], edx
	cmp	r14d, r15d
	jge	.L189
	movsx	rax, r14d
	mov	rdi, rax
	mov	QWORD PTR 16[rsp], rax
	vxorpd	xmm6, xmm6, xmm6
	lea	rax, result[rip]
	lea	rax, [rax+rdi*4]
	vcvtsi2sd	xmm0, xmm6, ebx
	mov	r13d, 15
	mov	QWORD PTR [rsp], rax
	sub	r13d, r14d
	lea	eax, -1[r14]
	mov	DWORD PTR 12[rsp], r13d
	mov	DWORD PTR 32[rsp], eax
	vmovsd	QWORD PTR 40[rsp], xmm0
	lea	r15, result[rip-4]
	mov	r12d, r14d
	.p2align 4,,10
	.p2align 3
.L201:
	movsx	rsi, edx
	sal	rsi, 4
	lea	rax, start_a[rip]
	add	rsi, rax
	mov	edi, 1
	mov	DWORD PTR 24[rsp], edx
	call	clock_gettime@PLT
	mov	edx, DWORD PTR 24[rsp]
	xor	eax, eax
	inc	edx
	mov	DWORD PTR timespec_i[rip], edx
	call	clear_total_iters
	test	ebx, ebx
	jle	.L191
	mov	eax, DWORD PTR 12[rsp]
	mov	edx, DWORD PTR 32[rsp]
	add	rax, QWORD PTR 16[rsp]
	lea	r13, 2[rdx]
	lea	rcx, result[rip+4]
	xor	r14d, r14d
	lea	rbp, [rcx+rax*4]
	mov	rax, r13
	mov	r13d, r14d
	mov	r14, rax
	.p2align 4,,10
	.p2align 3
.L194:
	mov	eax, 1
	test	r12d, r12d
	jle	.L193
	.p2align 4,,10
	.p2align 3
.L192:
	mov	DWORD PTR [r15+rax*4], eax
	inc	rax
	cmp	rax, r14
	jne	.L192
	cmp	r12d, 15
	jg	.L196
.L193:
	mov	rax, QWORD PTR [rsp]
	.p2align 4,,10
	.p2align 3
.L195:
	mov	DWORD PTR [rax], 0
	add	rax, 4
	cmp	rax, rbp
	jne	.L195
.L196:
	mov	esi, r12d
	lea	rdi, result[rip]
	call	shuffle
	inc	r13d
	mov	esi, r12d
	lea	rdi, result[rip]
	call	standard_bogosort
	cmp	ebx, r13d
	jne	.L194
.L191:
	xor	eax, eax
	call	grab_elapsed
	lea	rdi, .LC15[rip]
	vmovsd	QWORD PTR 24[rsp], xmm0
	call	puts@PLT
	vmovsd	xmm0, QWORD PTR 24[rsp]
	lea	rax, total_iters[rip]
	vdivsd	xmm2, xmm0, QWORD PTR 40[rsp]
	vmulsd	xmm2, xmm2, QWORD PTR .LC16[rip]
	xor	edx, edx
	.p2align 4,,10
	.p2align 3
.L198:
	add	rdx, QWORD PTR [rax]
	lea	rcx, total_iters[rip+64]
	add	rax, 8
	cmp	rcx, rax
	jne	.L198
	test	rdx, rdx
	js	.L199
	vxorpd	xmm4, xmm4, xmm4
	vcvtsi2sd	xmm1, xmm4, rdx
.L200:
	vdivsd	xmm1, xmm0, xmm1
	mov	edx, r12d
	mov	ecx, ebx
	lea	rsi, .LC17[rip]
	mov	edi, 1
	mov	eax, 3
	inc	r12d
	vmulsd	xmm1, xmm1, QWORD PTR .LC7[rip]
	call	__printf_chk@PLT
	xor	eax, eax
	dec	DWORD PTR timespec_i[rip]
	call	summarize_total_iters
	add	QWORD PTR [rsp], 4
	dec	DWORD PTR 12[rsp]
	inc	QWORD PTR 16[rsp]
	inc	DWORD PTR 32[rsp]
	cmp	DWORD PTR 36[rsp], r12d
	je	.L189
	mov	edx, DWORD PTR timespec_i[rip]
	jmp	.L201
.L199:
	mov	rax, rdx
	shr	rax
	and	edx, 1
	or	rax, rdx
	vxorpd	xmm5, xmm5, xmm5
	vcvtsi2sd	xmm1, xmm5, rax
	vaddsd	xmm1, xmm1, xmm1
	jmp	.L200
.L189:
	add	rsp, 56
	.cfi_def_cfa_offset 56
	pop	rbx
	.cfi_def_cfa_offset 48
	pop	rbp
	.cfi_def_cfa_offset 40
	pop	r12
	.cfi_def_cfa_offset 32
	pop	r13
	.cfi_def_cfa_offset 24
	pop	r14
	.cfi_def_cfa_offset 16
	lea	rdi, .LC19[rip]
	pop	r15
	.cfi_def_cfa_offset 8
	jmp	time_end
	.cfi_endproc
.LFE5607:
	.size	run_bogosort, .-run_bogosort
	.section	.rodata.str1.8
	.align 8
.LC20:
	.string	"\n\n\n\nRunning accelerated bogosort_nonzero, %i trials, minimum count %i, maximum count %i, thread count %i, AVX-512 %i\n"
	.align 8
.LC21:
	.string	"Nonzero elems,Iters,Time in s,One iter every ns,Average ms per case,Thread count"
	.section	.rodata.str1.1
.LC22:
	.string	"%i,%i,%f,%f,%f,%i\n"
.LC23:
	.string	"accelerated bogosort nonzero"
	.text
	.p2align 4
	.globl	run_accel_bogosort_nonzero
	.type	run_accel_bogosort_nonzero, @function
run_accel_bogosort_nonzero:
.LFB5608:
	.cfi_startproc
	endbr64
	push	r15
	.cfi_def_cfa_offset 16
	.cfi_offset 15, -16
	lea	rax, start_a[rip]
	mov	r15d, edx
	push	r14
	.cfi_def_cfa_offset 24
	.cfi_offset 14, -24
	mov	r14d, edi
	push	r13
	.cfi_def_cfa_offset 32
	.cfi_offset 13, -32
	push	r12
	.cfi_def_cfa_offset 40
	.cfi_offset 12, -40
	mov	r12d, ecx
	push	rbp
	.cfi_def_cfa_offset 48
	.cfi_offset 6, -48
	mov	ebp, r8d
	push	rbx
	.cfi_def_cfa_offset 56
	.cfi_offset 3, -56
	mov	ebx, esi
	sub	rsp, 72
	.cfi_def_cfa_offset 128
	movsx	rsi, DWORD PTR timespec_i[rip]
	mov	DWORD PTR 12[rsp], edi
	mov	r13, rsi
	sal	rsi, 4
	add	rsi, rax
	mov	edi, 1
	mov	DWORD PTR 52[rsp], edx
	mov	DWORD PTR 24[rsp], r8d
	call	clock_gettime@PLT
	sub	rsp, 8
	.cfi_def_cfa_offset 136
	push	rbp
	.cfi_def_cfa_offset 144
	inc	r13d
	mov	ecx, ebx
	mov	edx, r14d
	lea	rsi, .LC20[rip]
	mov	edi, 1
	xor	eax, eax
	mov	r9d, r12d
	mov	r8d, r15d
	mov	DWORD PTR timespec_i[rip], r13d
	call	__printf_chk@PLT
	mov	esi, DWORD PTR taskset_enabled[rip]
	pop	rdx
	.cfi_def_cfa_offset 136
	test	esi, esi
	lea	rax, .LC1[rip]
	lea	rdx, .LC0[rip]
	cmove	rdx, rax
	pop	rcx
	.cfi_def_cfa_offset 128
	xor	eax, eax
	lea	rsi, .LC2[rip]
	mov	edi, 1
	call	__printf_chk@PLT
	cmp	ebx, r15d
	jge	.L211
	movsx	rax, ebx
	mov	rdi, rax
	mov	QWORD PTR 32[rsp], rax
	lea	rax, result[rip]
	lea	rax, [rax+rdi*4]
	mov	QWORD PTR 16[rsp], rax
	vxorpd	xmm6, xmm6, xmm6
	mov	eax, 15
	sub	eax, ebx
	vcvtsi2sd	xmm0, xmm6, r14d
	mov	DWORD PTR 28[rsp], eax
	lea	eax, -1[rbx]
	mov	DWORD PTR 48[rsp], eax
	vmovsd	QWORD PTR 56[rsp], xmm0
	lea	r15, result[rip-4]
	.p2align 4,,10
	.p2align 3
.L222:
	movsx	rsi, DWORD PTR timespec_i[rip]
	lea	rax, start_a[rip]
	mov	rbp, rsi
	sal	rsi, 4
	add	rsi, rax
	mov	edi, 1
	call	clock_gettime@PLT
	lea	edx, 1[rbp]
	xor	eax, eax
	mov	DWORD PTR timespec_i[rip], edx
	call	clear_total_iters
	mov	eax, DWORD PTR 12[rsp]
	test	eax, eax
	jle	.L212
	mov	eax, DWORD PTR 28[rsp]
	mov	edx, DWORD PTR 48[rsp]
	add	rax, QWORD PTR 32[rsp]
	lea	rcx, result[rip+4]
	lea	r14, [rcx+rax*4]
	xor	ebp, ebp
	lea	r13, 2[rdx]
	mov	eax, ebx
	mov	rbx, r14
	mov	r14d, ebp
	mov	rbp, r13
	mov	r13d, eax
	.p2align 4,,10
	.p2align 3
.L213:
	mov	eax, 1
	test	r13d, r13d
	jle	.L218
	.p2align 4,,10
	.p2align 3
.L214:
	mov	DWORD PTR [r15+rax*4], eax
	inc	rax
	cmp	rax, rbp
	jne	.L214
	cmp	r13d, 15
	jg	.L215
.L218:
	mov	rax, QWORD PTR 16[rsp]
	.p2align 4,,10
	.p2align 3
.L216:
	mov	DWORD PTR [rax], 0
	add	rax, 4
	cmp	rax, rbx
	jne	.L216
.L215:
	mov	esi, 16
	lea	rdi, result[rip]
	call	shuffle
	mov	esi, DWORD PTR 24[rsp]
	mov	edi, r12d
	inc	r14d
	call	run_accel_bogosort
	cmp	DWORD PTR 12[rsp], r14d
	jne	.L213
	mov	ebx, r13d
.L212:
	xor	eax, eax
	call	grab_elapsed
	lea	rdi, .LC21[rip]
	vmovsd	QWORD PTR 40[rsp], xmm0
	call	puts@PLT
	vmovsd	xmm0, QWORD PTR 40[rsp]
	lea	rax, total_iters[rip]
	vdivsd	xmm2, xmm0, QWORD PTR 56[rsp]
	vmulsd	xmm2, xmm2, QWORD PTR .LC16[rip]
	xor	edx, edx
	.p2align 4,,10
	.p2align 3
.L219:
	add	rdx, QWORD PTR [rax]
	lea	rcx, total_iters[rip+64]
	add	rax, 8
	cmp	rcx, rax
	jne	.L219
	test	rdx, rdx
	js	.L220
	vxorpd	xmm4, xmm4, xmm4
	vcvtsi2sd	xmm1, xmm4, rdx
.L221:
	vdivsd	xmm1, xmm0, xmm1
	mov	ecx, DWORD PTR 12[rsp]
	mov	edx, ebx
	mov	r8d, r12d
	lea	rsi, .LC22[rip]
	mov	edi, 1
	mov	eax, 3
	inc	ebx
	vmulsd	xmm1, xmm1, QWORD PTR .LC7[rip]
	call	__printf_chk@PLT
	xor	eax, eax
	dec	DWORD PTR timespec_i[rip]
	call	summarize_total_iters
	add	QWORD PTR 16[rsp], 4
	dec	DWORD PTR 28[rsp]
	inc	QWORD PTR 32[rsp]
	inc	DWORD PTR 48[rsp]
	cmp	DWORD PTR 52[rsp], ebx
	jne	.L222
.L211:
	add	rsp, 72
	.cfi_remember_state
	.cfi_def_cfa_offset 56
	pop	rbx
	.cfi_def_cfa_offset 48
	pop	rbp
	.cfi_def_cfa_offset 40
	pop	r12
	.cfi_def_cfa_offset 32
	pop	r13
	.cfi_def_cfa_offset 24
	pop	r14
	.cfi_def_cfa_offset 16
	lea	rdi, .LC23[rip]
	pop	r15
	.cfi_def_cfa_offset 8
	jmp	time_end
.L220:
	.cfi_restore_state
	mov	rax, rdx
	shr	rax
	and	edx, 1
	or	rax, rdx
	vxorpd	xmm5, xmm5, xmm5
	vcvtsi2sd	xmm1, xmm5, rax
	vaddsd	xmm1, xmm1, xmm1
	jmp	.L221
	.cfi_endproc
.LFE5608:
	.size	run_accel_bogosort_nonzero, .-run_accel_bogosort_nonzero
	.section	.rodata.str1.8
	.align 8
.LC24:
	.string	"Running full %i-element accelerated bogosort, %i threads (AVX-512: %i)\n"
	.section	.rodata.str1.1
.LC25:
	.string	"Unsorted array: "
.LC26:
	.string	"Sorted array: "
	.section	.rodata.str1.8
	.align 8
.LC27:
	.string	"Accelerated bogosort %i elements"
	.text
	.p2align 4
	.globl	run_full_accel_bogosort
	.type	run_full_accel_bogosort, @function
run_full_accel_bogosort:
.LFB5609:
	.cfi_startproc
	endbr64
	push	rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	mov	ebp, edi
	mov	edi, 1
	push	rbx
	.cfi_def_cfa_offset 24
	.cfi_offset 3, -24
	sub	rsp, 120
	.cfi_def_cfa_offset 144
	movsx	rsi, DWORD PTR timespec_i[rip]
	mov	rax, QWORD PTR fs:40
	mov	QWORD PTR 104[rsp], rax
	xor	eax, eax
	mov	rbx, rsi
	lea	rax, start_a[rip]
	sal	rsi, 4
	add	rsi, rax
	call	clock_gettime@PLT
	inc	ebx
	xor	eax, eax
	mov	DWORD PTR timespec_i[rip], ebx
	call	clear_total_iters
	xor	r8d, r8d
	mov	ecx, ebp
	mov	edx, 16
	lea	rsi, .LC24[rip]
	mov	edi, 1
	xor	eax, eax
	call	__printf_chk@PLT
	mov	eax, DWORD PTR taskset_enabled[rip]
	lea	rdx, .LC0[rip]
	test	eax, eax
	lea	rax, .LC1[rip]
	cmove	rdx, rax
	lea	rsi, .LC2[rip]
	mov	edi, 1
	xor	eax, eax
	mov	DWORD PTR print_which_thread_found[rip], 1
	call	__printf_chk@PLT
	mov	eax, 1
	lea	rdx, result[rip-4]
	.p2align 4,,10
	.p2align 3
.L234:
	mov	DWORD PTR [rdx+rax*4], eax
	inc	rax
	cmp	rax, 17
	jne	.L234
	mov	esi, 16
	lea	rdi, result[rip]
	call	shuffle
	lea	rsi, .LC25[rip]
	mov	edi, 1
	xor	eax, eax
	call	__printf_chk@PLT
	mov	esi, 16
	lea	rdi, result[rip]
	call	print_arr
	xor	esi, esi
	mov	edi, ebp
	call	run_accel_bogosort
	lea	rsi, .LC26[rip]
	mov	edi, 1
	xor	eax, eax
	call	__printf_chk@PLT
	mov	esi, 16
	lea	rdi, result[rip]
	call	print_arr
	mov	rbp, rsp
	xor	eax, eax
	call	summarize_total_iters
	mov	r8d, 16
	lea	rcx, .LC27[rip]
	mov	edx, 100
	mov	esi, 1
	mov	rdi, rbp
	xor	eax, eax
	call	__sprintf_chk@PLT
	mov	rdi, rbp
	call	time_end
	mov	rax, QWORD PTR 104[rsp]
	xor	rax, QWORD PTR fs:40
	jne	.L240
	add	rsp, 120
	.cfi_remember_state
	.cfi_def_cfa_offset 24
	pop	rbx
	.cfi_def_cfa_offset 16
	pop	rbp
	.cfi_def_cfa_offset 8
	ret
.L240:
	.cfi_restore_state
	call	__stack_chk_fail@PLT
	.cfi_endproc
.LFE5609:
	.size	run_full_accel_bogosort, .-run_full_accel_bogosort
	.section	.rodata.str1.8
	.align 8
.LC28:
	.string	"Timing single-threaded accelerated bogosort with %i nonzero elements (AVX-512: %i)\n"
	.align 8
.LC29:
	.string	"accelerated bogosort 10 nonzero elements single threaded"
	.text
	.p2align 4
	.globl	single_threaded_iters
	.type	single_threaded_iters, @function
single_threaded_iters:
.LFB5610:
	.cfi_startproc
	endbr64
	push	rbx
	.cfi_def_cfa_offset 16
	.cfi_offset 3, -16
	mov	eax, 1
	lea	rdx, result[rip-4]
	.p2align 4,,10
	.p2align 3
.L242:
	mov	DWORD PTR [rdx+rax*4], eax
	inc	rax
	cmp	rax, 10
	jne	.L242
	mov	esi, 16
	lea	rdi, result[rip]
	mov	DWORD PTR result[rip+36], 0
	mov	QWORD PTR result[rip+40], 0
	mov	QWORD PTR result[rip+48], 0
	mov	QWORD PTR result[rip+56], 0
	call	shuffle
	movsx	rsi, DWORD PTR timespec_i[rip]
	lea	rax, start_a[rip]
	mov	rbx, rsi
	sal	rsi, 4
	add	rsi, rax
	mov	edi, 1
	call	clock_gettime@PLT
	inc	ebx
	lea	rsi, .LC25[rip]
	mov	edi, 1
	xor	eax, eax
	mov	DWORD PTR timespec_i[rip], ebx
	call	__printf_chk@PLT
	mov	esi, 16
	lea	rdi, result[rip]
	call	print_arr
	xor	ecx, ecx
	mov	edx, 9
	lea	rsi, .LC28[rip]
	mov	edi, 1
	xor	eax, eax
	call	__printf_chk@PLT
	xor	eax, eax
	call	clear_total_iters
	xor	edi, edi
	call	avx2_bogosort
	lea	rsi, .LC26[rip]
	mov	edi, 1
	xor	eax, eax
	call	__printf_chk@PLT
	lea	rdi, result[rip]
	mov	esi, 16
	call	print_arr
	xor	eax, eax
	call	summarize_total_iters
	lea	rdi, .LC29[rip]
	pop	rbx
	.cfi_def_cfa_offset 8
	jmp	time_end
	.cfi_endproc
.LFE5610:
	.size	single_threaded_iters, .-single_threaded_iters
	.p2align 4
	.globl	standard_battery_non_accel
	.type	standard_battery_non_accel, @function
standard_battery_non_accel:
.LFB5611:
	.cfi_startproc
	endbr64
	sub	rsp, 8
	.cfi_def_cfa_offset 16
	xor	eax, eax
	call	clear_total_iters
	mov	edx, 11
	xor	esi, esi
	mov	edi, 100
	call	run_bogosort
	xor	eax, eax
	call	clear_total_iters
	mov	edx, 7
	xor	esi, esi
	mov	edi, 100
	call	run_bogosort_nonzero
	xor	eax, eax
	call	clear_total_iters
	mov	edx, 8
	mov	esi, 7
	mov	edi, 20
	call	run_bogosort_nonzero
	xor	eax, eax
	add	rsp, 8
	.cfi_def_cfa_offset 8
	jmp	clear_total_iters
	.cfi_endproc
.LFE5611:
	.size	standard_battery_non_accel, .-standard_battery_non_accel
	.section	.rodata.str1.1
.LC30:
	.string	"standard battery"
	.text
	.p2align 4
	.globl	standard_battery
	.type	standard_battery, @function
standard_battery:
.LFB5612:
	.cfi_startproc
	endbr64
	push	r14
	.cfi_def_cfa_offset 16
	.cfi_offset 14, -16
	mov	edi, 1
	mov	r14d, 100
	push	r13
	.cfi_def_cfa_offset 24
	.cfi_offset 13, -24
	mov	r13d, 8
	push	r12
	.cfi_def_cfa_offset 32
	.cfi_offset 12, -32
	mov	r12d, 1
	push	rbp
	.cfi_def_cfa_offset 40
	.cfi_offset 6, -40
	xor	ebp, ebp
	push	rbx
	.cfi_def_cfa_offset 48
	.cfi_offset 3, -48
	sub	rsp, 208
	.cfi_def_cfa_offset 256
	movsx	rsi, DWORD PTR timespec_i[rip]
	mov	rax, QWORD PTR fs:40
	mov	QWORD PTR 200[rsp], rax
	xor	eax, eax
	mov	rbx, rsi
	lea	rax, start_a[rip]
	sal	rsi, 4
	add	rsi, rax
	call	clock_gettime@PLT
	inc	ebx
	xor	eax, eax
	mov	DWORD PTR timespec_i[rip], ebx
	call	standard_battery_non_accel
	movabs	rax, 17179869186
	mov	QWORD PTR 4[rsp], rax
	add	rax, 6
	mov	QWORD PTR 12[rsp], rax
	mov	QWORD PTR 20[rsp], rax
	movabs	rax, 51539607560
	mov	QWORD PTR 28[rsp], rax
	movabs	rax, 429496729700
	mov	QWORD PTR 52[rsp], rax
	movabs	rax, 85899346020
	mov	QWORD PTR 60[rsp], rax
	movabs	rax, 4294967316
	mov	QWORD PTR 68[rsp], rax
	movabs	rax, 12884901889
	mov	QWORD PTR 76[rsp], rax
	movabs	rax, 38654705672
	movabs	rdx, 47244640266
	mov	QWORD PTR 100[rsp], rax
	movabs	rax, 42949672969
	mov	QWORD PTR 108[rsp], rax
	mov	QWORD PTR 116[rsp], rdx
	mov	QWORD PTR 164[rsp], rax
	movabs	rcx, 47244640267
	movabs	rdx, 38654705664
	inc	rax
	mov	QWORD PTR 124[rsp], rcx
	mov	QWORD PTR 148[rsp], 0
	mov	QWORD PTR 156[rsp], rdx
	mov	QWORD PTR 172[rsp], rax
	xor	ebx, ebx
	jmp	.L250
	.p2align 4,,10
	.p2align 3
.L254:
	mov	r12d, DWORD PTR [rsp+rbx]
	mov	r13d, DWORD PTR 96[rsp+rbx]
	mov	ebp, DWORD PTR 144[rsp+rbx]
	mov	r14d, DWORD PTR 48[rsp+rbx]
.L250:
	cmp	r12d, 8
	jg	.L248
	mov	r8d, 1
	mov	ecx, r12d
	mov	edx, r13d
	mov	esi, ebp
	mov	edi, r14d
	mov	DWORD PTR taskset_enabled[rip], 0
	call	run_accel_bogosort_nonzero
	xor	eax, eax
	call	clear_total_iters
	mov	r8d, 1
	mov	ecx, r12d
	mov	edx, r13d
	mov	esi, ebp
	mov	edi, r14d
	mov	DWORD PTR taskset_enabled[rip], 1
	call	run_accel_bogosort_nonzero
	xor	eax, eax
	call	clear_total_iters
.L248:
	add	rbx, 4
	cmp	rbx, 32
	jne	.L254
	mov	rax, QWORD PTR 200[rsp]
	xor	rax, QWORD PTR fs:40
	jne	.L255
	add	rsp, 208
	.cfi_remember_state
	.cfi_def_cfa_offset 48
	pop	rbx
	.cfi_def_cfa_offset 40
	pop	rbp
	.cfi_def_cfa_offset 32
	pop	r12
	.cfi_def_cfa_offset 24
	pop	r13
	.cfi_def_cfa_offset 16
	lea	rdi, .LC30[rip]
	pop	r14
	.cfi_def_cfa_offset 8
	jmp	time_end
.L255:
	.cfi_restore_state
	call	__stack_chk_fail@PLT
	.cfi_endproc
.LFE5612:
	.size	standard_battery, .-standard_battery
	.section	.rodata.str1.8
	.align 8
.LC31:
	.string	"Max threads (accelerated only): %i\n"
	.align 8
.LC32:
	.string	"Shuffle lookup table size: %i\n"
	.section	.rodata.str1.1
.LC33:
	.string	"Compiled with taskset: %s\n"
.LC34:
	.string	"filled shuffles"
	.section	.text.startup,"ax",@progbits
	.p2align 4
	.globl	main
	.type	main, @function
main:
.LFB5613:
	.cfi_startproc
	endbr64
	push	rbx
	.cfi_def_cfa_offset 16
	.cfi_offset 3, -16
	imul	rax, QWORD PTR SEED[rip], 50
	mov	rdi, QWORD PTR stdout[rip]
	xor	ecx, ecx
	mov	edx, 2
	xor	esi, esi
	mov	QWORD PTR SEED[rip], rax
	call	setvbuf@PLT
	mov	edx, 8
	lea	rsi, .LC31[rip]
	mov	edi, 1
	xor	eax, eax
	call	__printf_chk@PLT
	mov	edx, 2048
	lea	rsi, .LC32[rip]
	mov	edi, 1
	xor	eax, eax
	call	__printf_chk@PLT
	lea	rdx, .LC0[rip]
	lea	rsi, .LC33[rip]
	mov	edi, 1
	xor	eax, eax
	call	__printf_chk@PLT
	movsx	rsi, DWORD PTR timespec_i[rip]
	lea	rax, start_a[rip]
	mov	rbx, rsi
	sal	rsi, 4
	add	rsi, rax
	mov	edi, 1
	call	clock_gettime@PLT
	inc	ebx
	xor	eax, eax
	mov	DWORD PTR timespec_i[rip], ebx
	call	fill_shuffles
	lea	rdi, .LC34[rip]
	call	time_end
	mov	edi, 8
	mov	DWORD PTR taskset_enabled[rip], 1
	call	run_full_accel_bogosort
	xor	eax, eax
	pop	rbx
	.cfi_def_cfa_offset 8
	ret
	.cfi_endproc
.LFE5613:
	.size	main, .-main
	.comm	elapsed,8,8
	.local	timespec_i
	.comm	timespec_i,4,4
	.comm	start_a,160,32
	.local	shifts
	.comm	shifts,256,32
	.local	shuffles
	.comm	shuffles,8,8
	.local	taskset_enabled
	.comm	taskset_enabled,4,4
	.comm	last_cleared,16,16
	.local	complete
	.comm	complete,4,4
	.local	measured_rdtsc
	.comm	measured_rdtsc,64,32
	.local	total_iters
	.comm	total_iters,64,32
	.local	print_which_thread_found
	.comm	print_which_thread_found,4,4
	.comm	result_mutex,40,32
	.local	result
	.comm	result,96,32
	.data
	.align 8
	.type	r, @object
	.size	r, 8
r:
	.quad	3
	.align 8
	.type	SEED, @object
	.size	SEED, 8
SEED:
	.quad	50
	.section	.rodata.cst8,"aM",@progbits,8
	.align 8
.LC7:
	.long	0
	.long	1104006501
	.section	.rodata.cst32,"aM",@progbits,32
	.align 32
.LC10:
	.long	0
	.long	0
	.long	1
	.long	2
	.long	3
	.long	4
	.long	5
	.long	6
	.section	.rodata.cst8
	.align 8
.LC16:
	.long	0
	.long	1083129856
	.ident	"GCC: (Ubuntu 9.4.0-1ubuntu1~20.04.1) 9.4.0"
	.section	.note.GNU-stack,"",@progbits
	.section	.note.gnu.property,"a"
	.align 8
	.long	 1f - 0f
	.long	 4f - 1f
	.long	 5
0:
	.string	 "GNU"
1:
	.align 8
	.long	 0xc0000002
	.long	 3f - 2f
2:
	.long	 0x3
3:
	.align 8
4:

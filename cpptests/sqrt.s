	.file	1 "sqrt.cpp"
	.section .mdebug.abi32
	.previous
	.nan	legacy
	.module	fp=xx
	.module	nooddspreg
	.module	arch=mips32
	.abicalls
	.text
	.align	2
	.globl	main
$LFB0 = .
	.cfi_startproc
	.set	nomips16
	.set	nomicromips
	.ent	main
	.type	main, @function
main:
	.frame	$fp,40,$31		# vars= 8, regs= 5/0, args= 0, gp= 8
	.mask	0x400f0000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$sp,$sp,-40
	.cfi_def_cfa_offset 40
	sw	$fp,36($sp)
	sw	$19,32($sp)
	sw	$18,28($sp)
	sw	$17,24($sp)
	sw	$16,20($sp)
	.cfi_offset 30, -4
	.cfi_offset 19, -8
	.cfi_offset 18, -12
	.cfi_offset 17, -16
	.cfi_offset 16, -20
	move	$fp,$sp
	.cfi_def_cfa_register 30
	move	$16,$0
	bne	$16,$0,$L2
	nop

	move	$2,$0
	b	$L3
	nop

$L2:
	li	$2,1			# 0x1
	bne	$16,$2,$L4
	nop

	li	$2,1			# 0x1
	b	$L3
	nop

$L4:
	move	$18,$0
	li	$17,1			# 0x1
	li	$2,2			# 0x2
	sw	$2,8($fp)
	b	$L5
	nop

$L6:
	addu	$19,$18,$17
	move	$18,$17
	move	$17,$19
	lw	$2,8($fp)
	addiu	$2,$2,1
	sw	$2,8($fp)
$L5:
	lw	$2,8($fp)
	sltu	$2,$2,$16
	bne	$2,$0,$L6
	nop

	move	$2,$0
$L3:
	move	$sp,$fp
	.cfi_def_cfa_register 29
	lw	$fp,36($sp)
	lw	$19,32($sp)
	lw	$18,28($sp)
	lw	$17,24($sp)
	lw	$16,20($sp)
	addiu	$sp,$sp,40
	.cfi_restore 16
	.cfi_restore 17
	.cfi_restore 18
	.cfi_restore 19
	.cfi_restore 30
	.cfi_def_cfa_offset 0
	jr	$31
	nop

	.set	macro
	.set	reorder
	.end	main
	.cfi_endproc
$LFE0:
	.size	main, .-main
	.ident	"GCC: (Debian 12.2.0-14) 12.2.0"
	.section	.note.GNU-stack,"",@progbits

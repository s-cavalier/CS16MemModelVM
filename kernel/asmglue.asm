# small glue function to force the compiler to handle the stack appropriately in tandem with the hardware's automatic trap functionality

.text
.set noreorder
.globl main
main:
    # all registers are init to 0, so we set the non-zeroed registers as necessary

    la $t0, top_kstack              # t0 = &top_kstack (double ptr)
    lw $sp, 0($t0)                  # sp = *(&k_sp) = *($t0)
    # la $t0, __global_pointer      # t0 = &__gp (double ptr)
    # lw $gp, 0($t0)                # gp = *(&__gp) = *($t0)

    jal cppmain
    nop

    # run first proc

    la $a0, currentThread
    lw $a0, 0($a0)

    j run_process
    nop

.globl kernel_trap
kernel_trap:
    
    # Two routes: we are either currently in a nested exception or not
    # If in a nested exception, we need to use the old $sp register value (so don't change anything) to not corrupt the stack after we return
    # Otherwise, we need to switch to the top_kstack variable.

  

    la $k0, exceptionDepth 
    lbu $k0, 0($k0)

    move $k1, $sp # save the stack pointer to k1, regardless

    bgtz $k0, .Lkernel_trap_nestedException # if bgtz > 0, we're in a nested exception, so we just use the previous $sp
    nop

    la $k0, top_kstack
    lw $sp, 0($k0)

.Lkernel_trap_nestedException:

    # load registers. $k1 has the "old" $sp value, which maybe be == $k1 or != $k1 based on if we're in a nested exception or not
    
    addiu $sp, $sp, -144 # 31 * 4 (registers) + 2 * 4 (hi/lo) + 3 * 4 (epc/status/cause)
    
    .set noat
    sw $at, 0($sp)
    sw $v0, 4($sp)
    sw $v1, 8($sp)
    sw $a0, 12($sp)
    sw $a1, 16($sp)
    sw $a2, 20($sp)
    sw $a3, 24($sp)
    sw $t0, 28($sp)
    sw $t1, 32($sp)
    sw $t2, 36($sp)
    sw $t3, 40($sp)
    sw $t4, 44($sp)
    sw $t5, 48($sp)
    sw $t6, 52($sp)
    sw $t7, 56($sp)
    sw $s0, 60($sp)
    sw $s1, 64($sp)
    sw $s2, 68($sp)
    sw $s3, 72($sp)
    sw $s4, 76($sp)
    sw $s5, 80($sp)
    sw $s6, 84($sp)
    sw $s7, 88($sp)
    sw $t8, 92($sp)
    sw $t9, 96($sp)

    # skip k0: 100, k1: 104

    sw $gp, 108($sp)
    sw $k1, 112($sp) # recall that $k1 stores the "old" $sp
    sw $fp, 116($sp)
    sw $ra, 120($sp)
    .set at

    mfhi $t0
    mflo $t1

    sw $t0, 124($sp) # hi
    sw $t1, 128($sp) # lo

    mfc0 $t0, $14 #epc
    mfc0 $t1, $12 #status
    mfc0 $t2, $13 #cause

    sw $t0, 132($sp)
    sw $t1, 136($sp)
    sw $t2, 140($sp)

    # increment exceptionDepth
    la $t0, exceptionDepth
    lbu $t1, 0($t0)
    addiu $t1, $t1, 1
    sb $t1, 0($t0)

    # put addr of ctx struct at $k0
    move $k0, $sp

    jal handleTrap
    nop

    la $a0, currentThread
    lw $a0, 0($a0)

    # unwind stack (since currentThread->regCtx holds the registers, we can do this safely)
    addiu $sp, $sp, 136

    # reduce exceptionDepth by one
    la $t0, exceptionDepth
    lbu $t1, 0($t0)
    addiu $t1, $t1, -1
    sb $t1, 0($t0)

    # PCB::RegisterContext is at offset 0, so we can just "pass" the currentThread ptr to run_process
    j run_process
    nop

# $a0 holds a pointer to a RegisterContext class. 
.globl run_process
run_process:

    # Similar to kernel_trap, we have two paths based on exception depth.
    # If exception depth > 0, we are staying in the kernel and need to use jr to jump to the desired spot instead of calling eret to maintain privliges.
    # Otherwise, we're returning to a user process and can call eret.

    # first, restore registers

    move $k0, $a0

    # do this before restoring the rest of the vars so temporary registers are available

    lw $t0, 132($k0)
    lw $t1, 136($k0)
    lw $t2, 140($k0)

    # lets be sure the EXL bit is enabled so we can maintain privilige. If we eret later, hardware will flip EXL for us
    ori $t1, $t1, 2

    mtc0 $t0, $14 #epc
    mtc0 $t1, $12 #status
    mtc0 $t2, $13 #cause

    move $k1, $t0 # save epc to $k1 in case of nested exception later

    # hi/lo regs
    lw $t0, 124($k0) # hi 
    lw $t1, 128($k0) # lo

    mthi $t0
    mtlo $t1

    .set noat
    lw $at, 0($k0)
    lw $v0, 4($k0)
    lw $v1, 8($k0)
    lw $a0, 12($k0)
    lw $a1, 16($k0)
    lw $a2, 20($k0)
    lw $a3, 24($k0)
    lw $t0, 28($k0)
    lw $t1, 32($k0)
    lw $t2, 36($k0)
    lw $t3, 40($k0)
    lw $t4, 44($k0)
    lw $t5, 48($k0)
    lw $t6, 52($k0)
    lw $t7, 56($k0)
    lw $s0, 60($k0)
    lw $s1, 64($k0)
    lw $s2, 68($k0)
    lw $s3, 72($k0)
    lw $s4, 76($k0)
    lw $s5, 80($k0)
    lw $s6, 84($k0)
    lw $s7, 88($k0)
    lw $t8, 92($k0)
    lw $t9, 96($k0)

    # skip k0: 100, k1: 104

    lw $gp, 108($k0)
    lw $sp, 112($k0) # recall that $k1 stores the "old" $sp
    lw $fp, 116($k0)
    lw $ra, 120($k0)
    .set at

    # two paths:

    la $k0, exceptionDepth
    lbu $k0, 0($k0)

    bgtz $k0, .Lrun_process_nestedret
    nop

    # not much else todo!    

    eret
    nop

.Lrun_process_nestedret:

    # nested exception, use jr $k1 to prevent losing priviliges even if returning to kernel still

    jr $k1

    nop



.set reorder



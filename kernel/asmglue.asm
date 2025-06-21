

# small glue function to force the compiler to handle the stack appropriately in tandem with the hardware's automatic trap functionality

.text
.set noreorder
.globl main
main:
    # all registers are init to 0, so we set the non-zeroed registers as necessary

    la $t0, k_sp                # t0 = &k_sp (double ptr)
    lw $sp, 0($t0)              # sp = *(&k_sp) = *($t0)
    # la $t0, __global_pointer    # t0 = &__gp (double ptr)
    # lw $gp, 0($t0)              # gp = *(&__gp) = *($t0)

    jal cppmain

    # Kill machine if we return here
    li $t0, 1
    addiu $sp, $sp, -4  # reduce stack by 4

    sw $t0, 0($sp)      # *sp = 1

    move $a0, $sp       # a0 = sp
                        # v0 doesn't matter, we don't care ab return just care ab killing the process
    .word 0x3f          # VMTUNNEL with HALT argument, should kill the system


.globl run_process
run_process:

    

    # since we won't be back until we hit a trap, make sure k_sp is set correctly
    la $t0, k_sp
    sw $sp, 0($t0)

    # first load the cop0 registers, epc, status, cause
    lw $t0, 124($a0)
    lw $t1, 128($a0)
    lw $t2, 132($a0)

    mtc0 $t0, $14 # epc <- will jump to here after eret is called
    # mtc0 $t1, $12 # status figure this out later
    mtc0 $t2, $13 # cause    

    .set noat
    lw $at, 0($a0)
    lw $v0, 4($a0)
    lw $v1, 8($a0)

    # a0 holds the address, replace it later

    lw $a1, 16($a0)
    lw $a2, 20($a0)
    lw $a3, 24($a0)
    lw $t0, 28($a0)
    lw $t1, 32($a0)
    lw $t2, 36($a0)
    lw $t3, 40($a0)
    lw $t4, 44($a0)
    lw $t5, 48($a0)
    lw $t6, 52($a0)
    lw $t7, 56($a0)
    lw $s0, 60($a0)
    lw $s1, 64($a0)
    lw $s2, 68($a0)
    lw $s3, 72($a0)
    lw $s4, 76($a0)
    lw $s5, 80($a0)
    lw $s6, 84($a0)
    lw $s7, 88($a0)
    lw $t8, 92($a0)
    lw $t9, 96($a0)
    lw $k0, 100($a0)
    lw $k1, 104($a0)
    lw $gp, 108($a0)
    lw $sp, 112($a0)
    lw $fp, 116($a0)
    lw $ra, 120($a0)

    # finally set $a0 correctly
    lw $a0, 12($a0)
    .set at

    eret


.globl kernel_trap
kernel_trap:
    
    # get k_sp first
    la $k0, k_sp            # k0 = &k_sp (double ptr)
    lw $k0, 0($k0)          # k0 = *(&k_sp) (ptr to top of stack)
    addiu $k0, $k0, -136    # reduce k0 (current stack pointer) by 136 to alloc enough space for the trap frame

    # save $sp first
    sw $sp, 112($k0)
    move $sp, $k0 # then store k0 into sp

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
    sw $k0, 100($sp)
    sw $k1, 104($sp)
    sw $gp, 108($sp)

    # lw $sp, 112($k0)

    sw $fp, 116($sp)
    sw $ra, 120($sp)

    # save cop0 values (EXL bit always on?)

    mfc0 $t0, $14 # epc figure this out later
    mfc0 $t1, $12 # status
    mfc0 $t2, $13 # cause

    sw $t0, 124($sp)
    sw $t1, 128($sp)
    sw $t2, 132($sp)
    .set at

    jal handleTrap

    # we can assume that the $sp now as the same as right before the jal by MIPS calling convention
    # so we unwind the stack now as well

    # move sp into k0 so we can freely update sp later
    move $k0, $sp

    lw $t0, 124($sp)
    lw $t1, 128($sp)
    lw $t2, 132($sp)

    mtc0 $t0, $14 # epc <- will jump to here after eret is called
    # mtc0 $t1, $12 # status figure this out later
    mtc0 $t2, $13 # cause

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

    # lw $k0, 100($k0) <- has sp, ignore restoration

    lw $k1, 104($k0)
    lw $gp, 108($k0)
    lw $sp, 112($k0)
    lw $fp, 116($k0)
    lw $ra, 120($k0)

    eret
.set reorder



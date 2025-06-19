

# small glue function to force the compiler to handle the stack appropriately in tandem with the hardware's automatic trap functionality

.text

.globl main
main:
    # all registers are init to 0, so we set the non-zeroed registers as necessary

    la $t0, k_sp                # t0 = &k_sp (double ptr)
    lw $sp, 0($t0)              # sp = *(&k_sp) = *($t0)
    la $t0, __global_pointer    # t0 = &__gp (double ptr)
    lw $gp, 0($t0)              # gp = *(&__gp) = *($t0)

    jal cppmain

    # Kill machine if we return here
    li $t0, 1
    addiu $sp, $sp, -4  # reduce stack by 4

    sw $t0, 0($sp)      # *sp = 1

    move $a0, $sp       # a0 = sp
                        # v0 doesn't matter, we don't care ab return just care ab killing the process
    .word 0x3f          # VMTUNNEL with HALT argument, should kill the system


.globl kernel_trap
kernel_trap:
    
    ### By now, we assume hardware has run the trap frame for us and everything is store at some offset + reg[$sp] <-- could be done here, but hardware is faster


    jal handleTrap

    eret

    # If we just wrote this as inline asm, we might've had some weird behavior with some epilouge stack stuff after eret bc the compiler normally doesn't recognize eret



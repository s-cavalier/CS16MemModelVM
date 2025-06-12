

# small glue function to force the compiler to handle the stack appropriately in tandem with the hardware's automatic trap functionality

.text
.globl
kernel_trap:
    
    ### By now, we assume hardware has run the trap frame for us and everything is store at some offset + reg[$sp] <-- could be done here, but hardware is faster
    ### So we just jal to handleTrap so the compiler's use of stack doesn't interfere.
    ### So now instead of calling kernel::ereturn() (possibly weird behavior) we just return from the kernel (unless we halt the machine)
    ### We have full gaurantee that the stack is well handled.

    jal handleTrap

    eret

    # If we just wrote this as inline asm, we might've had some weird behavior with some epilouge stack stuff after eret bc the compiler normally doesn't recognize eret



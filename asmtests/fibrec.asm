.text
.globl main

main:
    li $a0, 0         # Compute fib(4)
    jal fib

    move $a0, $v0
    li $v0, 1
    syscall

    li $v0, 10
    syscall

fib:
    # --- prologue: make room for RA, saved n, and one slot for t1
    addi $sp, $sp, -12
    sw   $ra, 8($sp)
    sw   $a0, 4($sp)
    # 0($sp) is free for us to store fib(n-1)

    # base case?
    slti $t0, $a0, 2
    bne $t0, $0, fib_base

    # --- recursive case ---
    # compute fib(n-1)
    addi $a0, $a0, -1
    jal  fib
    move $t1, $v0           # t1 = fib(n-1)
    sw   $t1, 0($sp)        # spill t1 on stack

    # compute fib(n-2)
    lw   $a0, 4($sp)        # restore original n
    addi $a0, $a0, -2
    jal  fib
    move $t2, $v0           # t2 = fib(n-2)

    # recover fib(n-1) and sum
    lw   $t1, 0($sp)
    add  $v0, $t1, $t2
    j    fib_epilogue

fib_base:
    # for n = 0 or 1, just return n
    lw   $v0, 4($sp)

fib_epilogue:
    # --- epilogue: restore RA and deallocate frame
    lw   $ra, 8($sp)
    addi $sp, $sp, 12
    jr   $ra
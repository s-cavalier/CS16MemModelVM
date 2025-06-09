main:
    li $a0, 0

    jal fib

    move $a0, $v0
    li $v0, 1
    syscall

    li $v0, 10
    syscall


fib:
    addiu   $sp, $sp, -40
    sw      $ra, 36($sp)
    sw      $fp, 32($sp)
    move    $fp, $sp
    sw      $a0, 40($fp)
    lw      $v0, 40($fp)
    bgez    $v0, x1
    nop

    li      $v0, -1
    b       x6
    nop

x1:
    lw      $v0, 40($fp)
    bnez    $v0, x2
    nop

    move    $v0, $zero
    b       x6
    nop

x2:
    sw      $zero, 16($fp)
    li      $v0, 1
    sw      $v0, 20($fp)
    li      $v0, 2
    sw      $v0, 24($fp)
    b       x4
    nop

x3:
    lw      $v0, 20($fp)
    sw      $v0, 28($fp)
    lw      $v1, 20($fp)
    lw      $v0, 16($fp)
    addu    $v0, $v1, $v0
    sw      $v0, 20($fp)
    lw      $v0, 28($fp)
    sw      $v0, 16($fp)
    lw      $v0, 24($fp)
    addiu   $v0, $v0, 1
    sw      $v0, 24($fp)

x4:
    lw      $v1, 24($fp)
    lw      $v0, 40($fp)
    slt     $v0, $v0, $v1
    beqz    $v0, x3
    nop

x5:
    lw      $v0, 20($fp)
x6:
    move    $sp, $fp
    lw      $ra, 36($sp)
    lw      $fp, 32($sp)
    addiu   $sp, $sp, 40
    jr      $ra
    nop



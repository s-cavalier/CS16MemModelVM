    .text
    .globl main

main:
    # Allocate 16 bytes on the stack
    addiu $sp, $sp, -16
    move  $t0, $sp      # $t0 = test buffer base address

    # --- Test 1: Store 0xAB (171) ---
    li    $t1, 0xAB
    sb    $t1, 0($t0)   # store to buffer[0]

    # LBU → result: 171 (unsigned)
    lbu   $t2, 0($t0)   # $t2 should be 171

    # LB → result: -85 (signed)
    lb    $t3, 0($t0)   # $t3 should be -85

    # --- Test 2: Store 100 ---
    li    $t1, 100
    sb    $t1, 1($t0)   # store to buffer[1]

    # LBU → result: 100
    lbu   $t4, 1($t0)   # $t4 should be 100

    # LB → result: 100
    lb    $t5, 1($t0)   # $t5 should be 100

    # End of test — restore stack
    addiu $sp, $sp, 16

    # Loop forever so you can inspect:
    # $t2 = LBU 0($t0)
    # $t3 =  LB 0($t0)
    # $t4 = LBU 1($t0)
    # $t5 =  LB 1($t0)
halt:
    li $v0, 10
    syscall

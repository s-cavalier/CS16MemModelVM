    .text
    main:
        # --- Test 2: Immediate arithmetic only ---
        addi $s0, $zero, -3     # s0 = -3
        addi $s1, $s0, 7        # s1 = 4
        ori  $s2, $s1, 3        # s2 = 4 | 3 = 7
        andi $s3, $s1, 2        # s3 = 4 & 2 = 0
        slti $s4, $s0, 0        # s4 = (-3 < 0) ? 1 : 0 = 1

        # exit
        li   $v0, 10
        syscall

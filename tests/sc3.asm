    .text
    main:
        # --- Test 4: Signed‐branch tests (bltz, bgez), no loops ---
        # Negative‐value test
        addi $t0, $zero, -8     # t0 = –8
        bltz $t0, NegLabel      # branch if t0<0 → should take
        addi $t1, $zero, 0      # (skipped)
    NegLabel:
        addi $t1, $zero, 11     # t1 = 11

        # Positive‐value test
        addi $t0, $zero, 9      # t0 = 9
        bgez $t0, PosLabel      # branch if t0≥0 → should take
        addi $t2, $zero, 0      # (skipped)
    PosLabel:
        addi $t2, $zero, 22     # t2 = 22

        # exit
        li   $v0, 10
        syscall

.text
main:
    # Initialize test values
    li $t0, 10       # $t0 = 10
    li $t1, 3        # $t1 = 3
    li $t2, -10      # $t2 = -10
    li $t3, 4        # $t3 = 4

    # MULT test: 10 * 3 = 30
    mult $t0, $t1
    mflo $t4         # $t4 = 30 (LO)
    mfhi $t5         # $t5 = 0  (HI, since no overflow)

    # DIV test: 10 / 3
    div $t0, $t1
    mflo $t6         # $t6 = 3  (quotient)
    mfhi $t7         # $t7 = 1  (remainder)

    # DIV with negative number: -10 / 4
    div $t2, $t3
    mflo $s0         # $s0 = -2
    mfhi $s1         # $s1 = -2 (remainder)

    # Manual HI/LO setting test
    li $t8, 1234
    li $t9, 5678
    mthi $t8         # HI = 1234
    mtlo $t9         # LO = 5678
    mfhi $s2         # $s2 = 1234
    mflo $s3         # $s3 = 5678

    # End of program 
end:
    li $v0, 10
    syscall

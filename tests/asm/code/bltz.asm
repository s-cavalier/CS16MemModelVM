    .text
    .globl main

main:
    # ---- Test BGTZ ----
    # Set $t0 to a positive value
    addi   $t0, $zero, 10      # $t0 = 10

    # If $t0 > 0, branch to POSITIVE
    bgtz   $t0, POSITIVE

    # If branch not taken, set $s0 = 1
    addi   $s0, $zero, 1
    j      TEST_BLEZ

POSITIVE:
    # Branch taken path: set $s0 = 2
    addi   $s0, $zero, 2

TEST_BLEZ:
    # ---- Test BLEZ ----
    # Set $t1 to a non-positive value (zero in this case)
    addi   $t1, $zero, 0       # $t1 = 0

    # If $t1 <= 0, branch to NONPOSITIVE
    blez   $t1, NONPOSITIVE

    # If branch not taken, set $s1 = 3
    addi   $s1, $zero, 3
    j      END

NONPOSITIVE:
    # Branch taken path: set $s1 = 4
    addi   $s1, $zero, 4

END:
    # Halt (simulator can stop here, or you can insert an infinite loop)
    li $v0, 10
    syscall

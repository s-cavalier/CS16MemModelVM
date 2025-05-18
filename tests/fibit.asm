.text
.globl main

main:
    # Set n (Fibonacci index) in $s0
    li $s0, 0         # You can change this value to compute another Fibonacci number

    # Check if n == 0
    beq $s0, $0, fib_zero

    # Check if n == 1
    li $t0, 1
    beq $s0, $t0, fib_one

    # Initialize
    li $t1, 0          # fib(0)
    li $t2, 1          # fib(1)
    li $t3, 2          # i = 2

loop_start:
    # if i > n, done
    bgt $t3, $s0, done

    # fib(i) = fib(i-1) + fib(i-2)
    add $t4, $t1, $t2   # $t4 = next Fibonacci number

    # update fib(i-2) and fib(i-1)
    move $t1, $t2
    move $t2, $t4

    # i++
    addi $t3, $t3, 1

    # loop
    j loop_start

fib_zero:
    li $a0, 0
    j print

fib_one:
    li $a0, 1
    j print

done:
    # $t2 has the final Fibonacci number
    move $a0, $t2

print:
    li $v0, 1          # syscall for print_int
    syscall

    li $v0, 10         # syscall for exit
    syscall

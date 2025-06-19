

.text
main:
    li $v0, 5
    syscall

    addiu $a0, $v0, 100
    li $v0, 1
    syscall

    li $v0, 10
    syscall
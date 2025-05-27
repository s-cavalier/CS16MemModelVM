

.text
main:
    li $t0, 0x40490EA6
    li $t1, 0x402DF3B6
    sw $t0, -4($sp)
    sw $t1, -8($sp)
    lwc1 $f1, -4($sp)   # pi
    lwc1 $f2, -8($sp)   # e

    add.s $f3, $f2, $f1 # e + pi
    sub.s $f4, $f2, $f1 # e - pi
    mul.s $f5, $f2, $f1 # e * pi
    div.s $f6, $f2, $f1 # e / pi

    li $v0, 10
    syscall
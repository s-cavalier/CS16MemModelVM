
# keep f0 as 0

.text
main:
    li $t0, 0x40490EA6     # Initial float value in binary (pi for right now)
    li $t1, 0x3727C5AC     # load error (set to 1e-3)
    li $t2, 0x40000000     # load const (2.0)
    sw $t0, -4($sp)
    sw $t1, -8($sp)
    sw $t2, -12($sp)
    lwc1 $f1, -4($sp)      # f1 = x
    lwc1 $f2, -8($sp)      # f2 = err
    lwc1 $f3, -12($sp)     # f3 = 2.0

    c.le.s $f1, $f0        # if value <= 0, exit
    bc1t exit

    div.s $f4, $f1, $f3    # f4 = guess = x / 2.0

while_start:
    mul.s $f5, $f4, $f4    # guess^2
    sub.s $f5, $f5, $f1    # guess^2 - x
    abs.s $f5, $f5         # abs(guess^2 - x)
    c.lt.s $f2, $f5        # err < abs(guess^2 - x)
    bc1f exit

    div.s $f5, $f1, $f4    # x / guess
    add.s $f5, $f5, $f4    # guess + x / guess
    div.s $f4, $f5, $f3    # (guess + x / guess) / 2.0
    j while_start

exit:
    add.s $f12, $f4, $f0
    li $v0, 2
    syscall

    li $a0, 10
    li $v0, 11
    syscall

    li $v0, 10
    syscall
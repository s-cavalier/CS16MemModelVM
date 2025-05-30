.data
hello: .asciiz "Hello world!\n"

.text
main:
    la $a0, hello       # 21080001
    li $v0, 4           # 24090001
    syscall             # 1128fffd

    li $v0, 10          #
    syscall             
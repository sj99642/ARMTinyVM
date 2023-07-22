// 5
// 0b101000 >> 3 = 0b101 = 5
.global main

main:
    mov r1, #40
    lsr r0, r1, #3
    b _exit

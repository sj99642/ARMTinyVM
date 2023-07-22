// 40
// 0b101 << 3 = 0b101000 = 40
.global main

main:
    mov r1, #5
    lsl r0, r1, #3
    b _exit

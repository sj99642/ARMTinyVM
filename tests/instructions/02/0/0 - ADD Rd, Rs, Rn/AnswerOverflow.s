// 2
// -5 + 7 = 2 (0xFFFFFFFB + 0x00000007 = 0x00000002)
.global main

main:
    ldr r1, =0xFFFFFFFB
    ldr r2, =7
    add r0, r1, r2
    b _exit

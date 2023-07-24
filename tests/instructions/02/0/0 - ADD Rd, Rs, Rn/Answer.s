// 7
// 4 + 3 = 7
.global main

main:
    mov r1, #4
    mov r2, #3
    add r0, r1, r2
    b _exit

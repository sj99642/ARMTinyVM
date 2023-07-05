.global main

main:
    ldr r0, =0x7FFFFFFF
    mov r1, #1
    add r0, r0, r1
    bl is_status_n
    b _exit

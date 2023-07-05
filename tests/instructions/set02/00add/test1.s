.global main

// We're checking that doing an ADD on 0x7FFFFFFF + 1 sets the N flag
main:
    ldr r0, =0x7FFFFFFF
    mov r1, #1
    add r0, r0, r1
    bl is_status_n
    b _exit

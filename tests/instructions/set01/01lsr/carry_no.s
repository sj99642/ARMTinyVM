.global main

// Start with 0b10, and shift by 1, giving 1 and not carrying anything off the right
main:
    mov r0, #2
    lsr r0, r0, #1
    bl is_status_c
    b _exit

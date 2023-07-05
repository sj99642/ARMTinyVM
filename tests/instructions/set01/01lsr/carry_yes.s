.global main

// Start with 0b10, and shift by 2, giving 0 and carrying off to the right
main:
    mov r0, #2
    lsr r0, r0, #2
    bl is_status_c
    b _exit

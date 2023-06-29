.global _start

_start:
    ldr r0, =main
    add r0, #1
    bx r0

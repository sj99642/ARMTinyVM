.global _start

_start:
    // Jump to main function in Thumb mode (this code will be compiled in ARM mode)
    // Manually put the exit function in LR in case main tries to jump back
    ldr r0, =main+1
    ldr r1, =_exit+1
    mov lr, r1
    bx r0

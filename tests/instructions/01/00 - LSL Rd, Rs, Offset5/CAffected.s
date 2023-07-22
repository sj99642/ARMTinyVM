// 2
// Two tests, the first should carry, then second shouldn't (so 0b10)
.global main

// Each test puts either 0 or 1 in r0
// The main function collects these values together in r5

main:
    // Start with fresh r5
    mov r5, #0

    // Do test 1 and store result
    bl test1
    lsl r0, #1
    orr r5, r0

    // Do test 2 and store result
    bl test2
    orr r5, r0

    // Exit
    mov r0, r5
    b _exit

test1:
    push {lr}
    // Carry a digit off the left
    // 0x20000020 << 3 = 0x00000100, with carry
    ldr r1, =0x20000020
    lsl r1, #3
    bl is_status_c
    pop {r3}
    bx r3

test2:
    push {lr}
    // Don't carry a digit off the left
    // 0x20000020 << 2 = 0x80000080, with no carry
    ldr r1, =0x20000020
    lsl r1, #2
    bl is_status_c
    pop {r3}
    bx r3

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
    // Carry a digit off the right
    // 0b1100 >> 3 = 0, with a bit shifted off the right
    mov r1, #0x0C
    asr r1, #3
    bl is_status_c
    pop {r3}
    bx r3

test2:
    push {lr}
    // Don't carry a digit off the right
    // 0b100 >> 2 = 1, with no bit shifted off the right
    mov r1, #4
    asr r1, #2
    bl is_status_c
    pop {r3}
    bx r3

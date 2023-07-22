// 3
// Three tests: 0b011
// 1. Positive shifted right will stay positive: 0100 >> 1 = 0010
// 2. Negative shifted right will stay negative: 1000 >> 1 = 1100
// 3. Negative shifted right by 0 will stay negative: 1000 >> 0 = 1000
.global main

// Each test puts either 0 or 1 in r0
// The main function collects these values together in r5

main:
    // Start with fresh r5
    mov r5, #0

    // Start with 0x40000000 (positive) and shift right by 1 to stay positive
    // Put the argument to shift right by in r0, and we get the return value back in r0
    ldr r0, =0x40000000
    bl arithmetic_right_shift_by_1
    lsl r5, #1
    orr r5, r0

    // Start with 0x80000000 (negative) and shift right by 1 to stay positive
    ldr r0, =0x80000000
    bl arithmetic_right_shift_by_1
    lsl r5, #1
    orr r5, r0

    // Start with 0x80000000 (negative) and shift right by 0 to stay negative
    ldr r0, =0x80000000
    bl arithmetic_right_shift_by_0
    lsl r5, #1
    orr r5, r0

    // Exit
    mov r0, r5
    b _exit

// Takes the value in r0, right shifts it by 1, and sets r0 based on whether the N flag is set
arithmetic_right_shift_by_1:
    push {lr}
    asr r0, #1
    bl is_status_n
    pop {r3}
    bx r3

// Takes the value in r0, right shifts it by 0, and sets r0 based on whether the N flag is set
arithmetic_right_shift_by_0:
    push {lr}
    asr r0, #0
    bl is_status_n
    pop {r3}
    bx r3

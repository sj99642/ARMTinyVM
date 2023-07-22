// 11
// Four tests: 0b1011
// 1. Zero shifted to the right stays zero: 0000 >> 1 = 0000
// 2. Non-zero shifted to the right can stay non-zero: 0010 >> 1 = 0001
// 3. Non-zero shifted to the right can become zero: 0001 >> 1 = 0000
// 4. A negative number shifted to the right by 32 can become zero: (0x80000000 >> 16) >> 16 = 0
.global main

// Each test puts either 0 or 1 in r0
// The main function collects these values together in r5

main:
    // Start with fresh r5
    mov r5, #0

    // Start with 0 (zero) and shift right by 1 to stay zero
    // Put the argument to shift right by in r0, and we get the return value back in r0
    mov r0, #0
    bl right_shift_by_1
    lsl r5, #1
    orr r5, r0

    // Start with 0b10 (non-zero) and shift right by 1 to stay non-zero
    mov r0, #2
    bl right_shift_by_1
    lsl r5, #1
    orr r5, r0

    // Start with 1 (non-zero) and shift right by 1 to become zero
    mov r0, #1
    bl right_shift_by_1
    lsl r5, #1
    orr r5, r0

    // Start with 0x80000000 (non-zero, negative) and shift right by 32 to become zero
    ldr r0, =0x80000000
    bl right_shift_by_32
    lsl r5, #1
    orr r5, r0

    // Exit
    mov r0, r5
    b _exit

// Takes the value in r0, right shifts it by 1, and sets r0 based on whether the N flag is set
right_shift_by_1:
    push {lr}
    lsr r0, #1
    bl is_status_z
    pop {r3}
    bx r3

// Takes the value in r0, right shifts it by 32, and sets r0 based on whether the N flag is set
right_shift_by_32:
    push {lr}
    lsr r0, #16
    lsr r0, #16
    bl is_status_z
    pop {r3}
    bx r3

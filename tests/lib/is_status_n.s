.global main

// Expected final result is 0b1010

main:
    // Test if the is_status_n function works properly
    mov r0, #0
    bl test_1

    lsl r0, #1
    bl test_2

    lsl r0, #1
    bl test_3

    lsl r0, #1
    bl test_4

    b _exit


// Each of the following sets the LSB of r0 to the result of its own test

test_1: // We expect (0-1) to set the N register
    push {lr}
    mov r4, r0

    // Do the operation and test CPSR.N
    mov r0, #0
    sub r0, #1
    bl is_status_n

    // Create the final result by merging the old r0 with the new one
    orr r0, r4

    // Return to caller, whose address is on the stack
    pop {r4}
    bx r4


test_2: // We expect (1-0) to clear the N register
    push {lr}
    mov r4, r0

    // Do the operation and test CPSR.N
    mov r0, #1
    sub r0, #0
    bl is_status_n

    // Create the final result by merging the old r0 with the new one
    orr r0, r4

    // Return to caller, whose address is on the stack
    pop {r4}
    bx r4



test_3: // We expect 0x80000000 AND 0xFFFFFFFF to set the N register
    push {lr}
    mov r4, r0

    // Do the operation and test CPSR.N
    ldr r0, =0x80000000
    ldr r1, =0xFFFFFFFF
    and r0, r1
    bl is_status_n

    // Create the final result by merging the old r0 with the new one
    orr r0, r4

    // Return to caller, whose address is on the stack
    pop {r4}
    bx r4


test_4: // We expect 0x40000000 AND 0xFFFFFFFF to clear the N register
    push {lr}
    mov r4, r0

    // Do the operation and test CPSR.N
    ldr r0, =0x40000000
    ldr r1, =0xFFFFFFFF
    and r0, r1
    bl is_status_n

    // Create the final result by merging the old r0 with the new one
    orr r0, r4

    // Return to caller, whose address is on the stack
    pop {r4}
    bx r4

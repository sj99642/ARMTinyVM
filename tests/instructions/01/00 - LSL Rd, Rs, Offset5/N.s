// 9
// Four tests. 
// 1. Positive can become negative: 0010 << 2 should set N
// 2. Positive can stay positive:   0010 << 1 should clear N
// 3. Negative can become positive: 1000 << 1 should clear N
// 4. Negative can stay negative:   1100 << 1 should set N
.global main

// Each test puts either 0 or 1 in r0
// The main function collects these values together in r5

main:
    // Start with fresh r5
    mov r5, #0

    // Do test 1 and store result
    bl test1
    lsl r0, #3
    orr r5, r0

    // Do test 2 and store result
    bl test2
    lsl r0, #2
    orr r5, r0

    // Do test 3 and store result
    bl test3
    lsl r0, #1
    orr r5, r0

    // Do test 4 and store result
    bl test4
    orr r5, r0

    // Exit
    mov r0, r5
    b _exit

test1:
    push {lr}

    // Set the carry flag by shifting 0x20000000 left by 2
    ldr r1, =0x20000000
    lsl r1, #2

    # Check if N is set and return
    bl is_status_n
    pop {r3}
    bx r3

test2:
    push {lr}

    // Clear the carry flag by shifting 0x20000000 left by 1
    ldr r1, =0x20000000
    lsl r1, #1

    # Check if N is set and return
    bl is_status_n
    pop {r3}
    bx r3

test3:
    push {lr}

    // Clear the carry flag by shifting 0x80000000 left by 1
    ldr r1, =0x80000000
    lsl r1, #1

    # Check if N is set and return
    bl is_status_n
    pop {r3}
    bx r3

test4:
    push {lr}

    // Set the carry flag by shifting 0x80000000 left by 1, leaving 0x80000000
    ldr r1, =0xC0000000
    lsl r1, #1

    # Check if N is set and return
    bl is_status_n
    pop {r3}
    bx r3

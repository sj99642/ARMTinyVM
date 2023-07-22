// 2
// Two tests. The first sets the carry flag then does an LSR by 0, which should leave the carry flag set.
// The second does the same but starts with the carry flag clear. 
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

    // Set the carry flag by adding 1 to 0xFFFFFFFF
    ldr r1, =0xFFFFFFFF
    add r1, #1

    # With the carry flag now set, shift right by 0, which should leave C set
    asr r1, #0

    # Check if C is still set and return
    bl is_status_c
    pop {r3}
    bx r3

test2:
    push {lr}

    // Clear the carry flag by adding 1 to 0x7FFFFFFF (as an unsigned int, this isn't a wraparound)
    ldr r1, =0x7FFFFFFF
    add r1, #1

    # With the carry flag now set, shift right by 0, which should leave C clear
    asr r1, #0

    # Check if C is still set and return
    bl is_status_c
    pop {r3}
    bx r3

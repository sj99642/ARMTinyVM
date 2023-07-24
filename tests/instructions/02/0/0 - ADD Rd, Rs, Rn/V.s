// 6
// Three tests:
// 1. Carries but doesn't overflow (0xFFFFFFFF+1)
// 2. Carries and overflows (ox80000000+0x80000000)
// 3. Overflows but doesn't carry (0x7FFFFFFF+1)
// 4. Neither carries nor overflows (0xFFFFFFFE+1)
.global main

main:
    // Test 1
    ldr r0, =0xFFFFFFFF
    ldr r1, =1
    bl test
    mov r5, r0

    // Test 2
    ldr r0, =0x80000000
    ldr r1, =0x80000000
    bl test
    lsl r5, #1
    orr r5, r0

    // Test 3
    ldr r0, =0x7FFFFFFF
    ldr r1, =1
    bl test
    lsl r5, #1
    orr r5, r0

    // Test 4
    ldr r0, =0x7FFFFFFE
    ldr r1, =1
    bl test
    lsl r5, #1
    orr r5, r0

    // Exit
    mov r0, r5
    b _exit




// The test function accepts an int in r0 and r1. It adds them together, then sets r0 to either 0 or 1 depending on
// whether the overflow flag was set when adding the two together with the ADD Rd, Rs, Rn instruction
test:
    push {lr}
    add r0, r0, r1
    bl is_status_v
    pop {r3}
    bx r3

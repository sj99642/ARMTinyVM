// 56
// 1. Negative produced from two negatives (0xFFFFFFFF+0xFFFFFFFF=0xFFFFFFFE)
// 2. Negative produced from a negative and a positive (0xFFFFFFFE+1=0xFFFFFFFF)
// 3. Negative produced from two positives (0x7FFFFFFF+2=0x80000001)
// 4. Non-negative productd from two positives (1+2=3)
// 5. Non-negative produced from a positive and a negative (5+0xFFFFFFFD=2)
// 6. Non-negative produced from two negatives (0x80000000+0x80000000=0)

.global main

main:
    // Test 1
    ldr r0, =0xFFFFFFFF
    ldr r1, =0xFFFFFFFF
    bl test
    mov r5, r0

    // Test 2
    ldr r0, =0xFFFFFFFE
    ldr r1, =1
    bl test
    lsl r5, #1
    orr r5, r0

    // Test 3
    ldr r0, =0x7FFFFFFF
    ldr r1, =2
    bl test
    lsl r5, #1
    orr r5, r0

    // Test 4
    ldr r0, =1
    ldr r1, =2
    bl test
    lsl r5, #1
    orr r5, r0

    // Test 5
    ldr r0, =5
    ldr r1, =0xFFFFFFFD
    bl test
    lsl r5, #1
    orr r5, r0

    // Test 6
    ldr r0, =0x80000000
    ldr r1, =0x80000000
    bl test
    lsl r5, #1
    orr r5, r0

    // Exit
    mov r0, r5
    b _exit

    


// r0 and r1 contain ints
// test adds them together and returns 0 or 1 depending on if adding the parameters together sets the N flag
test:
    push {lr}
    add r0, r0, r1
    bl is_status_n
    pop {r3}
    bx r3

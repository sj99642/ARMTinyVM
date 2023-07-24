// 24
// 1. Zero produced from negative plus positive (0xFFFFFFFF+1=0)
// 2. Zero produced from negative plus negative (0x80000000+0x80000000=0)
// 3. Non-zero produced from negative plus negative (0xFFFFFFFF+0xFFFFFFFF=0xFFFFFFFE)
// 4. Non-zero produced from negative plus positive (0xFFFFFFFF+2=1)
// 5. Non-zero produced from positive plus positive (1+1=2)


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
    ldr r0, =0xFFFFFFFF
    ldr r1, =0xFFFFFFFF
    bl test
    lsl r5, #1
    orr r5, r0

    // Test 4
    ldr r0, =0xFFFFFFFF
    ldr r1, =2
    bl test
    lsl r5, #1
    orr r5, r0

    // Test 5
    ldr r0, =1
    ldr r1, =1
    bl test
    lsl r5, #1
    orr r5, r0

    // Exit
    mov r0, r5
    b _exit

    


// r0 and r1 contain ints
// test adds them together and returns 0 or 1 depending on if adding the parameters together sets the Z flag
test:
    push {lr}
    add r0, r0, r1
    bl is_status_z
    pop {r3}
    bx r3

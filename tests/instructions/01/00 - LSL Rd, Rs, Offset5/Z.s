// 5
// Three tests. 
// 1. Shifting zero sets Z
// 2. Shifting a non-zero so that it's non-zero clears Z
// 3. Shifting a non-zero so that it becomes zero sets Z
.global main

// Each test puts either 0 or 1 in r0
// The main function collects these values together in r5

main:
    // Start with fresh r5
    mov r5, #0

    // Do test 1 and store result
    bl test1
    lsl r0, #2
    orr r5, r0

    // Do test 2 and store result
    bl test2
    lsl r0, #1
    orr r5, r0

    // Do test 3 and store result
    bl test3
    orr r5, r0


    // Exit
    mov r0, r5
    b _exit

test1:
    push {lr}

    // Shift zero to the left, thus staying zero
    mov r1, #0
    lsl r1, #10

    # Check if Z is set and return
    bl is_status_z
    pop {r3}
    bx r3

test2:
    push {lr}

    // Shift a non-zero thing to the left so that it stays non-zero
    mov r1, #1
    lsl r1, #1

    # Check if Z is set and return
    bl is_status_z
    pop {r3}
    bx r3

test3:
    push {lr}

    // Shift a non-zero thing to the left so that it becomes zero
    mov r1, #2
    lsl r1, #31

    # Check if Z is set and return
    bl is_status_z
    pop {r3}
    bx r3

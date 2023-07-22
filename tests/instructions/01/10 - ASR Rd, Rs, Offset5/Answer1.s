// 255
// 1. Shifting a negative number right adds ones at the start: 0xFFFF0000 >> 4 = 0xFFFFF000
// Only 8 bits can be returned, so we will return the most significant byte (which should be 0xFF)

.global main

main:
    ldr r1, =0xFFFF0000
    asr r0, r1, #4

    // Get just the 8 most significant bits
    lsr r0, #24
    b _exit

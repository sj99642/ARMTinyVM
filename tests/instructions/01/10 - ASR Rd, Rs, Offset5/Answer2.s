// 7
// 1. Shifting a positive number right adds ones at the start: 0x7FFF0000 >> 4 = 0x07FFF000
// Only 8 bits can be returned, so we will return the most significant byte (which should be 0x07)

.global main

main:
    ldr r1, =0x7FFF0000
    asr r0, r1, #4

    // Get just the 8 most significant bits
    lsr r0, #24
    b _exit

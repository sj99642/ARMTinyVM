.global main

main:
    mov r0, #42   // Set the program exit value
    mov r7, #1    // Exit system call magic number
    swi     #0    // Interupt / syscall

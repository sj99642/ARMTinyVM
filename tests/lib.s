.global _exit

// Assumes that the desired exit code is already in r0
// Calls the system call to exit the program
.section .text._exit,"ax"
_exit:
    mov r7, #1    // Exit system call magic number
    swi     #0    // Interupt / syscall


// These are used for several functions
// They put a 0 or a 1 into r0, and then jump back to the link register
.section .text._yes,"ax"
_yes:
    mov r0, #1
    b _yes_no_ret

.section .text._no,"ax"
_no:
    mov r0, #0

.section .text._yes_no_ret,"ax"
_yes_no_ret:
    bx lr



.global is_status_n
// Function which sets r0 to 1 if n is set, or 0 otherwise, and jumps back to the link register
.section .text.is_status_n,"ax"
is_status_n:
    // bpl branches only if N is set (so the comparison is negative)
    // If that's the case, then jump to "_yes", which puts a 1 in r0 and returns to LR
    // Otherwise, so N is clear, jump to "_no", which puts a 0 in r0 and returns to LR
    bmi _yes
    b _no


.global is_status_z
// Function which sets r0 to 1 if z is set, or 0 otherwise, and jumps back to the link register
.section .text.is_status_z,"ax"
is_status_z:
    // bne branches only if Z is set (so the comparison is zero)
    // If that's the case, then jump to "_yes", which puts a 1 in r0 and returns to LR
    // Otherwise, so N is clear, jump to "_no", which puts a 0 in r0 and returns to LR
    beq _yes
    b _no


.global is_status_c
// Function which sets r0 to 1 if c is set, or 0 otherwise, and jumps back to the link register
.section .text.is_status_c,"ax"
is_status_c:
    // bne branches only if C is set (so a carry happened)
    // If that's the case, then jump to "_yes", which puts a 1 in r0 and returns to LR
    // Otherwise, so N is clear, jump to "_no", which puts a 0 in r0 and returns to LR
    bcs _yes
    b _no


.global is_status_v
// Function which sets r0 to 1 if v is set, or 0 otherwise, and jumps back to the link register
.section .text.is_status_v,"ax"
is_status_v:
    // bvs branches only if V is set (so an overflow happened)
    // If that's the case, then jump to "_yes", which puts a 1 in r0 and returns to LR
    // Otherwise, so N is clear, jump to "_no", which puts a 0 in r0 and returns to LR
    bvs _yes
    b _no


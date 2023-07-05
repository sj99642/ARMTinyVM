#include "ARMTinyVM.h"
#include "instruction_set.h"
#include <string.h>
#include <stdio.h>


// PRIVATE FUNCTION DECLARATIONS


void tliAddSubtract(VM_instance* vm, uint16_t instruction);
void tliMovCmpAddSubImmediate(VM_instance* vm, uint16_t instruction);
void tliHighRegOperations(VM_instance* vm, uint16_t instruction);
void tliSoftwareInterrupt(VM_instance* vm, uint16_t instruction);


#define i32_sign(n) ((n & 0x80000000) >> 31)


// PUBLIC FUNCTIONS


/**
 * Creates and returns a new VM instance, based on a set of interaction functions, and the initial stack pointer and
 * program counter.
 * @param instrs
 * @param initialStackPointer
 * @param initialProgramCounter
 * @return
 */
VM_instance VM_new(VM_interaction_instructions* instrs,
                   uint32_t initialStackPointer,
                   uint32_t initialProgramCounter)
{
    VM_instance ret;
    memset(&(ret.registers), 0, sizeof(ret.registers));
    vm_stack_pointer(&ret) = initialStackPointer;
    vm_program_counter(&ret) = initialProgramCounter;
    ret.cpsr = 0;
    ret.interactionInstructions = instrs;
    ret.finished = false;

    return ret;
}


/**
 * Take a VM instance and execute a single instruction, which is at the memory address pointed to by the current
 * stack pointer.
 * @param vm
 */
void VM_executeSingleInstruction(VM_instance* vm)
{
    // Cache the readByte function
    uint8_t (*const readByte)(uint32_t addr) = vm->interactionInstructions->readByte;

    // Get the 16-bit instruction
    // They're stored little-endian, so the lowest byte is the least significant bit
    uint16_t instruction = readByte(vm_program_counter(vm));
    instruction += readByte(vm_program_counter(vm)+1) << 8;

    printf("0x%04x@0x%08x : ", instruction, vm_program_counter(vm));

    // Now we have the instruction, we increment the program counter by 2 to go to the next instruction
    vm_program_counter(vm) += 2;

    // Decode the instruction and act accordingly
    if (istl_software_interrupt(instruction)) { // Matches first 8 bits to 11011111
        tliSoftwareInterrupt(vm, instruction);
    } else if (istl_hi_reg_operations(instruction)) { // Matches first 6 bits to 010001
        tliHighRegOperations(vm, instruction);
    } else if (istl_add_subtract(instruction)) { // Matches first 5 bits to 00011
        tliAddSubtract(vm, instruction);
    } else if (istl_mov_cmp_add_sub_imm(instruction)) { // Matches first 3 bits to 001
        tliMovCmpAddSubImmediate(vm, instruction);
    }
}


/**
 * Executes up to `maxInstructions` instructions, and returns the number which were actually executed. It will be
 * smaller than `maxInstructions` if the program finishes before then.
 * @param vm
 * @param maxInstructions
 * @return
 */
uint32_t VM_executeNInstructions(VM_instance* vm, uint32_t maxInstructions)
{
    uint32_t i;
    for (i = 0; i < maxInstructions; i++) {
        // Has the program completed?
        if (vm->finished) {
            return i;
        }

        // It hasn't completed
        // Run a single instruction
        VM_executeSingleInstruction(vm);
    }

    return i;
}


/**
 * Prints out the current state of a virtual machine's Zephyrs
 * @param vm
 */
void VM_print(VM_instance* vm)
{
    // 12 general purpose registers
    for (uint8_t r = 0; r < 13; r++) {
        printf("r%u = %u\n", r, vm->registers[r]);
    }

    // Explicitly print the special purpose registers
    printf("sp = %u\n", vm_stack_pointer(vm));
    printf("lr = %u\n", vm_link_register(vm));
    printf("pc = %u\n", vm_program_counter(vm));

    // CPSR register
    printf("CPSR = { .N = %u, .Z = %u, .C = %u, .V = %u }\n",
           vm_get_cpsr_n(vm),
           vm_get_cpsr_z(vm),
           vm_get_cpsr_c(vm),
           vm_get_cpsr_v(vm));
}


// PRIVATE FUNCTIONS


/**
 * Sets the N and Z comparison bits based on the given value.
 * N will be set if the most significant bit of `value` is 1
 * Z will be set if `value == 0`
 * @param value
 */
void compareSetNZ(VM_instance* vm, uint32_t value)
{
    // Set N if the result looks like a negative number
    if (value & 0x80000000) {
        vm_set_cpsr_n(vm);
    } else {
        vm_clr_cpsr_n(vm);
    }

    // Set V if the value is 0
    if (value == 0) {
        vm_set_cpsr_z(vm);
    } else {
        vm_clr_cpsr_z(vm);
    }
}


/**
 * If (a + b) carries - that is, when interpreted as unsigned ints, a+b loops round past 0, the carry bit (C) will be
 * set. Otherwise, the carry bit will be cleared.
 * @param vm
 * @param a
 * @param b
 */
void compareSetC(VM_instance* vm, uint32_t a, uint32_t b)
{
    // We know an overflow happened if a+b is somehow smaller than either a or b
    uint32_t sum = a+b;

    if ((sum < a) || (sum < b)) {
        vm_set_cpsr_c(vm);
    } else {
        vm_clr_cpsr_c(vm);
    }
}


/**
 * This function runs compareSetC, and also sets the overflow bit (V) appropriately. We know an overflow has happened if
 * the two operands have the same sign as each other, but the result of an addition has a different sign.
 */
 void compareSetCV(VM_instance* vm, uint32_t a, uint32_t b)
{
     uint32_t sum = a + b;

     // Set the carry bit appropriately
     compareSetC(vm, a, b);

     // Calculate the overflow bit
     if (i32_sign(a) == i32_sign(b)) {
         // a and b have the same sign
         // if sum has a different sign than this, we've had an overflow
         if (i32_sign(sum) == i32_sign(a)) {
             // Same sign, so no overflow
             vm_clr_cpsr_z(vm);
         } else {
             // Different sign, so overflow
             vm_set_cpsr_z(vm);
         }
     } else {
         // An overflow is impossible if the signs of the operands are different
         vm_clr_cpsr_z(vm);
     }
}


/**
 * Add/subtract
 * Documented as instruction 2 in the manual
 * @param vm
 * @param instruction
 */
void tliAddSubtract(VM_instance* vm, uint16_t instruction)
{
    // Decode the operation
    // 0 <= i <= 1
    // 0 <= op <= 1
    // 0 <= rn <= 7
    // 0 <= rs <= 7
    // 0 <= rd <= 7
    uint8_t i =  (uint8_t) ((instruction & 0b0000010000000000) >> 10);
    uint8_t op = (uint8_t) ((instruction & 0b0000001000000000) >> 9);
    uint8_t rn = (uint8_t) ((instruction & 0b0000000111000000) >> 6);
    uint8_t rs = (uint8_t) ((instruction & 0b0000000000111000) >> 3);
    uint8_t rd = (uint8_t)  (instruction & 0b0000000000000111);

    if (op == 0) {
        if (i == 0) {
            // ADD Rd, Rs, Rn
            // Rd := Rn + Rs
            // Safe because rd, rn and rs are no higher than 7
            printf("ADD r%u, r%u, r%u\n", rd, rs, rn);
            vm->registers[rd] = vm->registers[rn] + vm->registers[rs];
        } else {

        }
    } else {

    }
}


/**
 * Move/compare/add/subtract immediate.
 * Documented as instruction 3 in manual.
 * @param vm
 * @param instruction
 */
void tliMovCmpAddSubImmediate(VM_instance* vm, uint16_t instruction)
{
    // Decode the operation
    // 0 <= op <= 3
    // 0 <= rd <= 7
    // 0 <= offset <= 255
    uint8_t op =     (uint8_t) ((instruction & 0b0001100000000000) >> 11);
    uint8_t rd =     (uint8_t) ((instruction & 0b0000011100000000) >> 8);
    uint8_t offset = (uint8_t)  (instruction & 0b0000000011111111);

    if (op == 0b00) {
        // MOV Rd, #Offset
        // Moves the 8-bit immediate value `Offset` into Rd
        printf("MOV r%u, #%u\n", rd, offset);
        vm->registers[rd] = offset;
    } else if (op == 0b01) {
        // CMP Rd, #Offset
        // Compares the register with the 8-bit offset
    } else if (op == 0b10) {

    } else {

    }
}


/**
 * High register operations, encompassing instructions beginning with 010001.
 * Documented as instruction 5 in the manual.
 * @param vm
 * @param instruction
 */
void tliHighRegOperations(VM_instance* vm, uint16_t instruction)
{
    // Decode the pieces of the operation
    // 0 <= op <= 3
    // 0 <= h1_and_2 <= 3
    // 0 <= rs <= 7
    // 0 <= rd <= 7
    uint8_t op =       (uint8_t) ((instruction & 0b0000001100000000) >> 8);
    uint8_t h1_and_2 = (uint8_t) ((instruction & 0b0000000011000000) >> 6);
    uint8_t rs =       (uint8_t) ((instruction & 0b0000000000111000) >> 3);
    uint8_t rd =       (uint8_t)  (instruction & 0b0000000000000111);

    if (op == 0b00) {
        if (h1_and_2 == 0b01) {
            // ADD Rd, Hs
            // Sum together the values in a low register Rd and a high register Rs (8+Rs),
            // and store the result in low register Rd
            printf("ADD r%u, H%u", rd, 8+rs);
            vm->registers[rd] = vm->registers[rd] + vm->registers[8+rs];
        } else if (h1_and_2 == 0x10) {
            // ADD Hd, Rs
            // Sum together the values in high register Hd (8+Rd) and low register Rs
            // then store the result in Hd
            printf("ADD H%u, R%u", 8+rd, rs);
            vm->registers[8+rd] = vm->registers[rs];
        } else if (h1_and_2 == 0b11) {
            // ADD Hd, Hs
            // Sum together the values in the high registers Hd (8+Rd) and Hs (8+Rs)
            // then store the result in Hd
            printf("ADD H%u, H%u", 8+rd, 8+rs);
            vm->registers[8+rd] = vm->registers[8+rd] + vm->registers[8+rs];
        } else {
            printf("Invalid command %u", instruction);
            vm->finished = 0;
        }
    } else if (op == 0b01) {

    } else if (op == 0b10) {
        if (h1_and_2 == 0x01) {
            // MOV Rd, Hs
            // Moves a value from high register Hs (8+Rs) into low register Rd
            // rd <= 7 and (8+rs) <= 15, so this is safe
            printf("MOV r%u, r%u\n", rd, 8+rs);
            vm->registers[rd] = vm->registers[8+rs];
        } else if (h1_and_2 == 0x10) {

        } else if (h1_and_2 == 0x11) {

        } else {
            printf("Invalid command %u", instruction);
            vm->finished = 0;
        }
    } else {

    }
}


/**
 * Software interrupt
 * Documented as instruction 17 in manual
 * @param vm
 * @param instruction
 */
void tliSoftwareInterrupt(VM_instance* vm, uint16_t instruction)
{
    // Decode the instruction
    uint8_t value = instruction & 0x00FF;

    // Move the address of the next instruction into the link register
    vm_link_register(vm) = vm_program_counter(vm);

    // Trigger the interrupt
    vm->interactionInstructions->softwareInterrupt(vm, value);
}

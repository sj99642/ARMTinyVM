#include "ARMTinyVM.h"
#include "instruction_set.h"
#include <string.h>
#include <stdio.h>


// PRIVATE FUNCTION DECLARATIONS


void tliAddSubtract(VM_instance* vm, uint16_t instruction);
void tliMovCmpAddSubImmediate(VM_instance* vm, uint16_t instruction);
void tliHighRegOperations(VM_instance* vm, uint16_t instruction);
void tliSoftwareInterrupt(VM_instance* vm, uint16_t instruction);


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
           vm_cpsr_n(vm),
           vm_cpsr_z(vm),
           vm_cpsr_c(vm),
           vm_cpsr_v(vm));
}


// PRIVATE FUNCTIONS


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

    // Trigger the interrupt
    vm->interactionInstructions->softwareInterrupt(vm, value);
}

/*
 * Public interface for the Tiny ARM Virtual Machine. See README.md for usage.
*/

#ifndef ARMTINYVM_H
#define ARMTINYVM_H

#include <stdint.h>
#include <stdbool.h>

// Predefining VM_instance for use in VM_interaction_instructions
typedef struct VM_instance VM_instance;

/**
 * Behaviour which has to be provided to the VM for it to know how to interact with its environment
 * (reading and writing memory, and dealine with software interrupts).
 */
typedef struct VM_interaction_instructions {
    uint8_t (*const readByte)(uint32_t addr);
    void (*const writeByte)(uint32_t addr, uint8_t value);
    void (*const softwareInterrupt)(VM_instance* vm, uint8_t number);
} VM_interaction_instructions;


/**
 * Holds the registers and other information necessary to represent the state of the VM.
 */
typedef struct VM_instance {
    uint32_t registers[16];
    uint32_t cpsr;
    VM_interaction_instructions* interactionInstructions;
    bool finished;
} VM_instance;

// The following functions are used to access the special purpose registers, since they're
// in the array
#define vm_stack_pointer(vmptr)   ((vmptr)->registers[13])
#define vm_link_register(vmptr)   ((vmptr)->registers[14])
#define vm_program_counter(vmptr) ((vmptr)->registers[15])
#define vm_cpsr_n(vmptr)  ((((vmptr)->cpsr) & 0x80000000) >> 31)
#define vm_cpsr_z(vmptr)  ((((vmptr)->cpsr) & 0x40000000) >> 30)
#define vm_cpsr_c(vmptr)  ((((vmptr)->cpsr) & 0x20000000) >> 29)
#define vm_cpsr_v(vmptr)  ((((vmptr)->cpsr) & 0x10000000) >> 28)


VM_instance VM_new(VM_interaction_instructions* instrs,
                   uint32_t initialStackPointer,
                   uint32_t initialProgramCounter);
void VM_executeSingleInstruction(VM_instance* vm);
uint32_t VM_executeNInstructions(VM_instance* vm, uint32_t maxInstructions);
void VM_print(VM_instance* vm);


#endif // ARMTINYVM_H

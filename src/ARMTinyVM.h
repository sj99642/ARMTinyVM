/*
 * Public interface for the Tiny ARM Virtual Machine. See README.md for usage.
*/

#ifndef ARMTINYVM_H
#define ARMTINYVM_H

#include <stdint.h>
#include <stdbool.h>

/**
 * Behaviour which has to be provided to the VM for it to know how to interact with its environment (reading and writing
 * memory, and dealine with software interrupts).
 */
typedef struct VM_interaction_instructions {
    uint8_t (*const readByte)(uint32_t addr);
    void (*const writeByte)(uint32_t addr, uint8_t value);
    void (*const softwareInterrupt)(uint8_t number);
} VM_interaction_instructions;


/**
 * Holds the registers and other information necessary to represent the state of the VM.
 */
typedef struct VM_instance {
    uint32_t gen_registers[13];
    uint32_t stackPointer;
    uint32_t linkRegister;
    uint32_t programCounter;
    uint32_t cpsr;
    VM_interaction_instructions* interactionInstructions;
    bool finished;
} VM_instance;


VM_instance VM_new(VM_interaction_instructions* instrs,
                   uint32_t initialStackPointer,
                   uint32_t initialProgramCounter);
void VM_executeSingleInstructions(VM_instance* vm);
uint32_t VM_executeNInstructions(VM_instance* vm, uint32_t maxInstructions);


#endif // ARMTINYVM_H

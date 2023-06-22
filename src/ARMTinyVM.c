#include "ARMTinyVM.h"
#include <string.h>
#include <stdio.h>

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
    memset(&ret.gen_registers, 0, sizeof(ret.gen_registers));
    ret.stackPointer = initialStackPointer;
    ret.linkRegister = 0;
    ret.programCounter = initialProgramCounter;
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
void VM_executeSingleInstructions(VM_instance* vm)
{

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
    return 0;
}

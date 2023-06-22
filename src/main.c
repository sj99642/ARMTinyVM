#include "ARMTinyVM.h"
#include <stdio.h>

// FUNCTION DECLARATIONS
int main();
uint8_t readByte(uint32_t addr);
void writeByte(uint32_t addr, uint8_t value);
void softwareInterrupt(VM_instance* vm, uint8_t number);


// Static variables
#define MEM_SIZE 1024
#define HIGHEST_ADDRESS (MEM_SIZE-1)
uint8_t memory[MEM_SIZE];


// FUNCTION DEFINITIONS

int main()
{
    VM_interaction_instructions instrs = {
        .readByte = readByte,
        .writeByte = writeByte,
        .softwareInterrupt = softwareInterrupt
    };
    VM_instance vm = VM_new(&instrs, HIGHEST_ADDRESS, 0);

    return 0;
}


uint8_t readByte(uint32_t addr)
{
    if (addr < MEM_SIZE) {
        return memory[addr];
    } else {
        return 0xFF;
    }
}


void writeByte(uint32_t addr, uint8_t value)
{
    if (addr < MEM_SIZE) {
        memory[addr] = value;
    }
}


void softwareInterrupt(VM_instance* vm, uint8_t number)
{
    printf("Software interrupt: %u", number);
    vm->finished = true;
}

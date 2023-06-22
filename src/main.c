#include "ARMTinyVM.h"
#include <stdio.h>

// FUNCTION DECLARATIONS
int main();
uint8_t readByte(uint32_t addr);
void writeByte(uint32_t addr, uint8_t value);
void softwareInterrupt(VM_instance* vm, uint8_t number);


// Static variables
#define MEM_SIZE 256
uint8_t memory[MEM_SIZE] = {
    0x78, 0x46, 0x02, 0x21, 0x4a, 0x18, 0x00, 0xdf, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};


// FUNCTION DEFINITIONS

int main()
{
    VM_interaction_instructions instrs = {
        .readByte = readByte,
        .writeByte = writeByte,
        .softwareInterrupt = softwareInterrupt
    };
    VM_instance vm = VM_new(&instrs, MEM_SIZE, 0);
    uint32_t instrs_executed = VM_executeNInstructions(&vm, 10);
    printf("Number of instructions executed: %u\n", instrs_executed);
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
    printf("SWI #%u\n", number);
    printf("Software interrupt: %u\n", number);
    vm->finished = true;
}

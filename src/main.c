#include "ARMTinyVM.h"
#include "elf.h"
#include <stdio.h>
#include <string.h>

// FUNCTION DECLARATIONS
int main(int argc, char* argv[]);
uint8_t readByte(uint32_t addr);
void writeByte(uint32_t addr, uint8_t value);
void softwareInterrupt(VM_instance* vm, uint8_t number);


// FUNCTION DEFINITIONS

int main(int argc, char* argv[])
{
    // argv[1] should contain the filename of the ELF we're interested in
    if (argc < 2) {
        return 1;
    }
    char* elf_filename = argv[1];

    // Read the ELF
    Elf32_Ehdr header;
    FILE* file = fopen(elf_filename, "rb");
    if (file) {
        // Read the header
        fread(&header, sizeof(header), 1, file);

        // Check the first few bytes to see if it's really an ELF
        if (memcmp(header.ident, ELFMAG, SELFMAG) != 0) {
            // This is an invalid ELF file
            printf("ELF Identifier not found\n");
            return 1;
        }

        printf("Read file successfully\n");
        printf("ELF Identifier: %s\n", header.ident);
        printf("Architecture: %u\n", header.machine);
        printf("Entry point: %u\n", header.entry);

        // Close the file
        fclose(file);
    } else {
        printf("Unable to load file\n");
        return 1;
    }

    return 0;

//    VM_interaction_instructions instrs = {
//        .readByte = readByte,
//        .writeByte = writeByte,
//        .softwareInterrupt = softwareInterrupt
//    };
//    VM_instance vm = VM_new(&instrs, MEM_SIZE, 0);
//    uint32_t instrs_executed = VM_executeNInstructions(&vm, 10);
//    printf("Number of instructions executed: %u\n", instrs_executed);
//    VM_print(&vm);
//    return 0;
}

/*
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
*/

#include "ARMTinyVM.h"
#include "elf.h"
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) || defined(WIN64) || defined(_WIN64) || defined(__WIN64)
// On Windows, so using the local mmap because the system doesn't provide one
#include "mman.h"
#else
#include <sys/mann.h>
#endif

#define ELF_MAX_SIZE (1024*1024*1024)

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
    FILE* file = fopen(elf_filename, "rb");
    if (!file) {
        printf("Unable to load file\n");
        return 1;
    }

    // We've found a valid file
    // Now make a memory map so we can read the file as an array rather than as a stream
    char* elfContent = mmap(NULL, ELF_MAX_SIZE/1024, PROT_READ, MAP_SHARED, fileno(file), 0);
    if (elfContent == MAP_FAILED) {
        printf("Unable to map file into memory: error %u\n", errno);
        return errno;
    }

    // We can now address elfContent however we want
    Elf32_Ehdr* header = (Elf32_Ehdr*) &(elfContent[0]);
    printf("Read file successfully\n");
    printf("ELF Identifier: %s\n", header->ident);
    printf("Architecture: %u\n", header->machine);
    printf("Entry point: %u\n", header->entry);


    munmap(elfContent, ELF_MAX_SIZE);
    fclose(file);
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

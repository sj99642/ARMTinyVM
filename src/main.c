#include "ARMTinyVM.h"
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) || defined(WIN64) || defined(_WIN64) || defined(__WIN64)
#include "win_elf.h"
#else
#include <elf.h>
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

    // Find the size of the file by going to the end, seeing where we are, and going back to the beginning
    fseek(file, 0L, SEEK_END);
    long elfSize = ftell(file);
    rewind(file);

    // Allocate memory big enough for the full file
    char* elfContent = malloc(elfSize);

    // Read the whole file into that buffer
    fread(elfContent, elfSize, 1, file);

    // We can now address elfContent however we want
    Elf32_Ehdr* header = (Elf32_Ehdr*) &(elfContent[0]);
    printf("Read file successfully\n");
    printf("ELF Identifier: %s\n", header->e_ident);
    printf("ELF Type: 0x%x\n", header->e_type);
    printf("Architecture: %u\n", header->e_machine);
    printf("ELF Version: %u\n", header->e_version);
    printf("Entry point: %u\n", header->e_entry);
    printf("Offset in ELF of program header table: %u\n", header->e_phoff);
    printf("Offset in ELF of section header table: %u\n", header->e_shoff);
    printf("Flags: 0x%x\n", header->e_flags);
    printf("Size of this header: %u\n", header->e_ehsize);
    printf("Size of a program header table entry: %u\n", header->e_phentsize);
    printf("Number of program headers: %u\n", header->e_phnum);
    printf("Size of a section header table entry: %u\n", header->e_shentsize);
    printf("Number of section headers: %u\n", header->e_shnum);
    printf("Index of section header table which contains section names: %u\n\n", header->e_shstrndx);


    // The program headers, containing the loading information, begin at offset e_phoff. There are e_phnum entries,
    // each of size e_phentsize.
    Elf32_Half programNum = 0;
    Elf32_Phdr* programHeader;
    do {
        // Find the current header based on the header num, the starting offset, and the size of each one
        programHeader = (Elf32_Phdr*) &(elfContent[header->e_phoff + (programNum * header->e_phentsize)]);

        // Print out its information
        printf("Program header %u\n", programNum);
        printf("Segment type: %u\n", programHeader->p_type);
        printf("Offset of segment in ELF file: %u\n", programHeader->p_offset);
        printf("Virtual address of segment in memory: 0x%x\n", programHeader->p_vaddr);
        printf("Physical address of segment in memory: 0x%x\n", programHeader->p_paddr);
        printf("Size of segment in ELF file: %u\n", programHeader->p_filesz);
        printf("Size of segment in memory: %u\n", programHeader->p_memsz);
        printf("Flags: 0x%x\n", programHeader->p_flags);
        printf("Alignment: %u\n\n", programHeader->p_align);

        // Move on to the next header
        ++programNum;
    } while (programNum < header->e_phnum);

    // Now go through the section headers in a similar manner
    Elf32_Half sectionNum = 0;
    Elf32_Shdr* sectionHeader;
    do {
        // Find the section header based on the offset of the first one, the size of each one and the number so far
        sectionHeader = (Elf32_Shdr*) &(elfContent[header->e_shoff + (sectionNum * header->e_shentsize)]);

        // Print its information
        printf("Section header %u\n", sectionNum);
        printf("Name is at .shstrtab offset: %u\n", sectionHeader->sh_name);
        printf("Type: 0x%x\n", sectionHeader->sh_type);
        printf("Flags: 0x%x\n", sectionHeader->sh_flags);
        printf("Virtual address (if loaded): 0x%x\n", sectionHeader->sh_addr);
        printf("Offset of section in ELF: 0x%x\n", sectionHeader->sh_offset);
        printf("Size of section in ELF: %u\n", sectionHeader->sh_size);
        printf("Section index link (meanings differ): %u\n", sectionHeader->sh_link);
        printf("Extra info (meanings differ): %u\n", sectionHeader->sh_info);
        printf("Alignment: %u\n", sectionHeader->sh_addralign);
        printf("Entry size (if applicable): %u\n\n", sectionHeader->sh_entsize);

        ++sectionNum;
    } while (sectionNum < header->e_shnum);

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

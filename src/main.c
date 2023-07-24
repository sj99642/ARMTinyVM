#include "ARMTinyVM.h"
#include <stdio.h>
#include <string.h>
#include <malloc.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) || defined(WIN64) || defined(_WIN64) || defined(__WIN64)
#include "win_elf.h"
#else
#include <elf.h>
#endif

#define MAX_NUM_SEGMENTS 10
#define STACK_START_ADDR 0xFFFFFFFC
#define MAX_STACK_SIZE 0x10000

// FUNCTION AND STRUCT DECLARATIONS
int main(int argc, char* argv[]);
uint8_t readByte(uint32_t addr);
void writeByte(uint32_t addr, uint8_t value);
void softwareInterrupt(VM_instance* vm, uint8_t number);


typedef struct runtimeSegment {
    uint32_t virtualStartAddress;
    uint32_t length;
    uint8_t* content;
} runtimeSegment;


// VARIABLES FOR EXECUTION

runtimeSegment segments[MAX_NUM_SEGMENTS];
uint8_t numAllocatedSegments = 0;
int32_t exitCode = -0x40000000;


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

    // Read the whole file into that buffer, then close the file
    fread(elfContent, elfSize, 1, file);
    fclose(file);

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
        printf("============ Program header %u ============\n", programNum);
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


    // Before we process the sections, we need to find a pointer to the section containing the names
    // The header for this section begins at index e_shstrndx in the section header table
    // This section header's sh_offset field then gives the index in the file at which the content of the section begins
    // From then on, each section will give an index (in bytes) into this field for where their own name begins (stored
    // as a null-terminated string)
    Elf32_Shdr* stringSectionHeader = (Elf32_Shdr*) &(elfContent[header->e_shoff + (header->e_shstrndx * header->e_shentsize)]);
    char* sectionNameList = &(elfContent[stringSectionHeader->sh_offset]);


    // Now go through the section headers in a similar manner
    Elf32_Half sectionNum = 0;
    Elf32_Shdr* sectionHeader;
    do {
        // Find the section header based on the offset of the first one, the size of each one and the number so far
        sectionHeader = (Elf32_Shdr*) &(elfContent[header->e_shoff + (sectionNum * header->e_shentsize)]);

        // Print its information
        printf("============ Section header %u ============\n", sectionNum);
        printf("Name is at .shstrtab offset: %u\n", sectionHeader->sh_name);
        printf("Section name: %s\n", &(sectionNameList[sectionHeader->sh_name]));
        printf("Type: 0x%x\n", sectionHeader->sh_type);
        printf("Flags: 0x%x\n", sectionHeader->sh_flags);
        printf("Virtual address (if loaded): 0x%x\n", sectionHeader->sh_addr);
        printf("Offset of section in ELF: 0x%x\n", sectionHeader->sh_offset);
        printf("Size of section in ELF: %u\n", sectionHeader->sh_size);
        printf("Section index link (meanings differ): %u\n", sectionHeader->sh_link);
        printf("Extra info (meanings differ): %u\n", sectionHeader->sh_info);
        printf("Alignment: %u\n", sectionHeader->sh_addralign);
        printf("Entry size (if applicable): %u\n", sectionHeader->sh_entsize);

        if (sectionHeader->sh_flags & SHF_ALLOC) {
            // Needs to actually be loaded at runtime
            segments[numAllocatedSegments].virtualStartAddress = sectionHeader->sh_addr;
            segments[numAllocatedSegments].length = sectionHeader->sh_size;
            segments[numAllocatedSegments].content = malloc(sectionHeader->sh_size);

            // Load the content of the section from the ELF file into the newly allocated memory
            memcpy(
                    segments[numAllocatedSegments].content,
                    &(elfContent[sectionHeader->sh_offset]),
                    sectionHeader->sh_size
            );
            numAllocatedSegments++;

            printf("Allocated and loaded virtual memory segment starting at 0x%x, with size %u\n\n", sectionHeader->sh_addr, sectionHeader->sh_size);
        } else {
            printf("Not to be loaded\n\n");
        }

        ++sectionNum;
    } while (sectionNum < header->e_shnum);

    // One last segment to allocate is for the stack to live in, which will be full of zeroes
    segments[numAllocatedSegments].virtualStartAddress = STACK_START_ADDR - MAX_STACK_SIZE;
    segments[numAllocatedSegments].length = MAX_STACK_SIZE;
    segments[numAllocatedSegments].content = (uint8_t*) calloc(MAX_STACK_SIZE, 1);
    numAllocatedSegments++;

    // Now we have set up the memory space and can begin to execute the code
    VM_instance vm = VM_new(&readByte, &writeByte, &softwareInterrupt, STACK_START_ADDR, header->e_entry & 0xFFFFFFFE);
    uint32_t instrsExecuted = VM_executeNInstructions(&vm, 1000);
    printf("\n\n\n\nExecuted %u instructions\n", instrsExecuted);
    VM_print(&vm);

    return (int8_t) exitCode;
}

/**
 * Tries to find the byte pointed to by this virtual memory address, by searching in the `segments` array. Returns NULL
 * if none found.
 * @param addr
 * @return
 */
uint8_t* getVirtualMemoryByte(uint32_t addr)
{
    for (uint8_t i = 0; i < numAllocatedSegments; i++) {
        // Is this address included in this segment?
        if ((addr >= segments[i].virtualStartAddress) && (addr < (segments[i].virtualStartAddress + segments[i].length))) {
            // The byte is in this segment
            // Calculate its offset and find a pointer to that byte
            uint32_t offset = addr - segments[i].virtualStartAddress;
            return &(segments[i].content[offset]);
        }
    }

    // No matches found
    return NULL;
}


/**
 * Reads a byte from the given virtual address. Returns 0xFF if the address is invalid.
 * @param addr
 * @return
 */
uint8_t readByte(uint32_t addr)
{
    uint8_t* bytePtr = getVirtualMemoryByte(addr);
    if (bytePtr == NULL) {
        return 0xFF;
    } else {
        return *bytePtr;
    }
}


/**
 * Writes a byte to the given virtual address. Does nothing if the address is invalid.
 * @param addr
 * @param value
 */
void writeByte(uint32_t addr, uint8_t value)
{
    uint8_t* bytePtr = getVirtualMemoryByte(addr);
    if (bytePtr != NULL) {
        *bytePtr = value;
    }
}


void softwareInterrupt(VM_instance* vm, uint8_t number)
{
    printf("Software interrupt: %u\n", number);

    if (number == 0) {
        // This is intended as a system call
        // Check in r7 to see which system call
        if (vm->registers[7] == 1) {
            // We're being asked for the exit() syscall, and the exit code will be in r0
            exitCode = (int32_t) vm->registers[0];
        }
    }

    vm->finished = true;
}

#include "ARMTinyVM.h"
#include <stdbool.h>
#define NULL ((void*) 0)

uint8_t readByte(uint32_t addr);
void writeByte(uint32_t addr, uint8_t value);
void softwareInterrupt(VM_instance* vm, uint8_t number);


// Terrible practice, but I'm going to include a text file, which actually contains C code
// This text file will contain a hard-coded copy of the content of each section, written as array. In addition, it
// will define:
//  A preprocessor directive called NUM_SEGMENTS, defining the number of virtual memory segments
//  uint32_t startPositions[NUM_SEGMENTS] - An array of the start positions in virtual memory of each segment
//  uint32_t lengths[NUM_SEGMENTS] - An array containing the length of each segment
//  _Bool writable[NUM_SEGMENTS] - An array containing whether each segment is writable
//  uint8_t* segments[NUM_SEGMENTS] - An array of pointers to the predefined segments
//  uint32_t entryAddress - The virtual address to start the PC with
#include "avr_program.txt"

#define STACK_SIZE 1024UL
#define INITIAL_STACK_POINTER 0xFFFFFFF8UL
uint8_t stack[STACK_SIZE];

int exitCode = 0;


int main()
{
    VM_interaction_instructions instrs = {
            .readByte = readByte,
            .writeByte = writeByte,
            .softwareInterrupt = softwareInterrupt
    };
    VM_instance vm = VM_new(&instrs, INITIAL_STACK_POINTER, 0);
    return exitCode;
}


/**
 * Translates a virtual memory byte into a pointer to where that byte really lives. Returns
 * @param vaddr
 * @return
 */
uint8_t* getVirtualMemoryByte(uint32_t vaddr, bool* byteWritable)
{
    // Go through each of the predefined segments to see if this is in there
    for (uint8_t i = 0; i < NUM_SEGMENTS; i++) {
        if (vaddr > startPositions[i] && vaddr < (startPositions[i] + lengths[i])) {
            // It's in this segment
            if (byteWritable) {
                // If not null, report whether this is a writable segment
                *byteWritable = writable[i];
            }
            return &(segments[i][vaddr - startPositions[i]]);
        }
    }

    // If it wasn't in one of those, was it in the stack?
    // Ignore the CLion message telling me that the second condition is always false
    if ((vaddr > (INITIAL_STACK_POINTER - STACK_SIZE)) && (vaddr < INITIAL_STACK_POINTER)) {
        // It's in the stack
        if (byteWritable) {
            *byteWritable = true;
        }
        return &stack[vaddr - (INITIAL_STACK_POINTER - STACK_SIZE)];
    }

    return NULL;
}


/**
 * Reads a byte from the given virtual address. Returns 0xFF if the address is invalid.
 * @param addr
 * @return
 */
uint8_t readByte(uint32_t addr)
{
    uint8_t* bytePtr = getVirtualMemoryByte(addr, NULL);
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
    bool byteWritable;
    uint8_t* bytePtr = getVirtualMemoryByte(addr, &byteWritable);
    if ((bytePtr != NULL) && byteWritable) {
        *bytePtr = value;
    }
}


void softwareInterrupt(VM_instance* vm, uint8_t number)
{
    if (number == 0) {
        // This is intended as a system call
        // Check in r7 to see which system call
        if (vm->registers[7] == 1) {
            // We're being asked for the exit() syscall, and the exit code will be in r0
            exitCode = vm->registers[0];
        }
    }

    vm->finished = true;
}


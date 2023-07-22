﻿#include "ARMTinyVM.h"
#include "instruction_set.h"
#include <string.h>
#include <stdio.h>


// PRIVATE FUNCTION DECLARATIONS


void tliMoveShiftedRegister(VM_instance* vm, uint16_t instruction);
void tliAddSubtract(VM_instance* vm, uint16_t instruction);
void tliMovCmpAddSubImmediate(VM_instance* vm, uint16_t instruction);
void tliALUOperations(VM_instance* vm, uint16_t instruction);
void tliHighRegOperations(VM_instance* vm, uint16_t instruction);
void tliPCRelativeLoad(VM_instance* vm, uint16_t instruction);
void tliLoadWithRegOffset(VM_instance* vm, uint16_t instruction);
void tliLoadStoreSignExtendedByte(VM_instance* vm, uint16_t instruction);
void tliLoadStoreWithImmediateOffset(VM_instance* vm, uint16_t instruction);
void tliLoadStoreHalfWord(VM_instance* vm, uint16_t instruction);
void tliSPRelativeLoad(VM_instance* vm, uint16_t instruction);
void tliLoadAddress(VM_instance* vm, uint16_t instruction);
void tliAddOffsetToSP(VM_instance* vm, uint16_t instruction);
void tliPushPopRegisters(VM_instance* vm, uint16_t instruction);
void tliMultipleLoadStore(VM_instance* vm, uint16_t instruction);
void tliConditionalBranch(VM_instance* vm, uint16_t instruction);
void tliSoftwareInterrupt(VM_instance* vm, uint16_t instruction);
void tliUnconditionalBranch(VM_instance* vm, uint16_t instruction);
void tliLongBranchWithLink(VM_instance* vm, uint16_t instruction);
uint32_t load(VM_instance* vm, uint32_t addr, uint8_t bytes);
void store(VM_instance* vm, uint32_t addr, uint32_t value, uint8_t bytes);


#define i32_sign(n) (((n) & 0x80000000) >> 31)
#define i16_sign(n) (((n) & 0x8000) >> 15)
#define i8_sign(n)  (((n) & 0x80) >> 7)


// PUBLIC FUNCTIONS


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
    memset(&(ret.registers), 0, sizeof(ret.registers));
    vm_stack_pointer(&ret) = initialStackPointer;
    vm_program_counter(&ret) = initialProgramCounter;
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
void VM_executeSingleInstruction(VM_instance* vm)
{
    // Cache the readByte function
    uint8_t (*const readByte)(uint32_t addr) = vm->interactionInstructions->readByte;

    // Get the 16-bit instruction
    // They're stored little-endian, so the lowest byte is the least significant bit
    uint16_t instruction = readByte(vm_program_counter(vm));
    instruction += readByte(vm_program_counter(vm)+1) << 8;

    printf("0x%04x@0x%08x : ", instruction, vm_program_counter(vm));

    // Now we have the instruction, we increment the program counter by 2 to go to the next instruction
    vm_program_counter(vm) += 2;

    // Decode the instruction and act accordingly
    uint8_t instrFirstByte = (instruction & 0xFF00) >> 8;
    void (*func)(VM_instance*, uint16_t) = tliMoveShiftedRegister;
    if (istl_move_shifted_reg(instrFirstByte)) {
        func = tliMoveShiftedRegister;
    } else if (istl_add_subtract(instrFirstByte)) {
        func = tliAddSubtract;
    } else if (istl_mov_cmp_add_sub_imm(instrFirstByte)) {
        func = tliMovCmpAddSubImmediate;
    } else if (istl_alu_operations(instrFirstByte)) {
        func = tliALUOperations;
    } else if (istl_hi_reg_operations(instrFirstByte)) {
        func = tliHighRegOperations;
    } else if (istl_pc_relative_load(instrFirstByte)) {
        func = tliPCRelativeLoad;
    } else if (istl_load_with_reg_offset(instrFirstByte)) {
        func = tliLoadWithRegOffset;
    } else if (istl_load_sgn_ext_byte(instrFirstByte)) {
        func = tliLoadStoreSignExtendedByte;
    } else if (istl_load_imm_offset(instrFirstByte)) {
        func = tliLoadStoreWithImmediateOffset;
    } else if (istl_load_halfword(instrFirstByte)) {
        func = tliLoadStoreHalfWord;
    } else if (istl_sp_relative_load(instrFirstByte)) {
        func = tliSPRelativeLoad;
    } else if (istl_load_address(instrFirstByte)) {
        func = tliLoadAddress;
    } else if (istl_add_offset_to_sp(instrFirstByte)) {
        func = tliAddOffsetToSP;
    } else if (istl_push_pop_registers(instrFirstByte)) {
        func = tliPushPopRegisters;
    } else if (istl_multiple_load_store(instrFirstByte)) {
        func = tliMultipleLoadStore;
    } else if (istl_conditional_branch(instrFirstByte)) {
        func = tliConditionalBranch;
    } else if (istl_software_interrupt(instrFirstByte)) {
        func = tliSoftwareInterrupt;
    } else if (istl_unconditional_branch(instrFirstByte)) {
        func = tliUnconditionalBranch;
    } else if (istl_long_branch_w_link(instrFirstByte)) {
        func = tliLongBranchWithLink;
    } else {
        // No matching operation
        printf("UNKNOWN INSTRUCTION %x\n", instruction);
        vm->finished = true;
        return;
    }

    // Call the chosen function
    func(vm, instruction);
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
    uint32_t i;
    for (i = 0; i < maxInstructions; i++) {
        // Has the program completed?
        if (vm->finished) {
            return i;
        }

        // It hasn't completed
        // Run a single instruction
        VM_executeSingleInstruction(vm);
    }

    return i;
}


/**
 * Prints out the current state of a virtual machine's Zephyrs
 * @param vm
 */
void VM_print(VM_instance* vm)
{
    // 12 general purpose registers
    for (uint8_t r = 0; r < 13; r++) {
        printf("r%u = %u\n", r, vm->registers[r]);
    }

    // Explicitly print the special purpose registers
    printf("sp = %u\n", vm_stack_pointer(vm));
    printf("lr = %u\n", vm_link_register(vm));
    printf("pc = %u\n", vm_program_counter(vm));

    // CPSR register
    printf("CPSR = { .N = %u, .Z = %u, .C = %u, .V = %u }\n",
           vm_get_cpsr_n(vm),
           vm_get_cpsr_z(vm),
           vm_get_cpsr_c(vm),
           vm_get_cpsr_v(vm));
}


/***********************************************************************************************************************
 * COMPARISONS
 **********************************************************************************************************************/


/**
 * Sets the N and Z comparison bits based on the given value.
 * N will be set if the most significant bit of `value` is 1
 * Z will be set if `value == 0`
 * @param value
 */
void compareSetNZ(VM_instance* vm, uint32_t value)
{
    // Set N if the result looks like a negative number
    if (value & 0x80000000) {
        vm_set_cpsr_n(vm);
    } else {
        vm_clr_cpsr_n(vm);
    }

    // Set V if the value is 0
    if (value == 0) {
        vm_set_cpsr_z(vm);
    } else {
        vm_clr_cpsr_z(vm);
    }
}


/**
 * If (a + b) carries - that is, when interpreted as unsigned ints, a+b loops round past 0, the carry bit (C) will be
 * set. Otherwise, the carry bit will be cleared.
 * @param vm
 * @param a
 * @param b
 */
void compareSetC(VM_instance* vm, uint32_t a, uint32_t b)
{
    // We know an overflow happened if a+b is somehow smaller than either a or b
    uint32_t sum = a+b;

    if ((sum < a) || (sum < b)) {
        vm_set_cpsr_c(vm);
    } else {
        vm_clr_cpsr_c(vm);
    }
}


/**
 * This function runs compareSetC, and also sets the overflow bit (V) appropriately. We know an overflow has happened if
 * the two operands have the same sign as each other, but the result of an addition has a different sign.
 */
 void compareSetCV(VM_instance* vm, uint32_t a, uint32_t b)
{
     uint32_t sum = a + b;

     // Set the carry bit appropriately
     compareSetC(vm, a, b);

     // Calculate the overflow bit
     if (i32_sign(a) == i32_sign(b)) {
         // a and b have the same sign
         // if sum has a different sign than this, we've had an overflow
         if (i32_sign(sum) == i32_sign(a)) {
             // Same sign, so no overflow
             vm_clr_cpsr_z(vm);
         } else {
             // Different sign, so overflow
             vm_set_cpsr_z(vm);
         }
     } else {
         // An overflow is impossible if the signs of the operands are different
         vm_clr_cpsr_z(vm);
     }
}


/***********************************************************************************************************************
 * LOADING AND STORING PRIMITIVES
 **********************************************************************************************************************/


uint32_t load(VM_instance* vm, uint32_t addr, uint8_t bytes)
{
    if (bytes == 1) {
        // Single byte
        return vm->interactionInstructions->readByte(addr);
    } else if (bytes == 2) {
        // Half word
        uint32_t value = vm->interactionInstructions->readByte(addr);
        value += vm->interactionInstructions->readByte(addr+1) << 8;
        return value;
    } else {
        // Full word
        uint32_t value = vm->interactionInstructions->readByte(addr);
        value += vm->interactionInstructions->readByte(addr+1) << 8;
        value += vm->interactionInstructions->readByte(addr+2) << 16;
        value += vm->interactionInstructions->readByte(addr+3) << 24;
        return value;
    }
}


void store(VM_instance* vm, uint32_t addr, uint32_t value, uint8_t bytes)
{
    if (bytes == 1) {
        // Single byte
        vm->interactionInstructions->writeByte(addr, (uint8_t) (value & 0x000000FF));
    } else if (bytes == 2) {
        // Half word
        vm->interactionInstructions->writeByte(addr,   (uint8_t) (value & 0x000000FF));
        vm->interactionInstructions->writeByte(addr+1, (uint8_t) (value & 0x0000FF00) >> 8);
    } else {
        // Full word
        vm->interactionInstructions->writeByte(addr,   (uint8_t) (value & 0x000000FF));
        vm->interactionInstructions->writeByte(addr+1, (uint8_t) (value & 0x0000FF00) >> 8);
        vm->interactionInstructions->writeByte(addr+2, (uint8_t) (value & 0x00FF0000) >> 16);
        vm->interactionInstructions->writeByte(addr+3, (uint8_t) (value & 0xFF000000) >> 24);
    }
}


/***********************************************************************************************************************
 * OPERATIONS
 **********************************************************************************************************************/


/**
 * Move shifted register
 * Documented as instruction 1 in the manual
 * Covers instructions with first byte 000XXXXX, except 00011XXX
 * @param vm
 * @param instruction
 */
void tliMoveShiftedRegister(VM_instance* vm, uint16_t instruction)
{
    // Decode the operation
    // 0 <= op <= 3, but 3 is invalid
    // 0 <= offset5 <= 31
    // 0 <= rs <= 7
    // 0 <= rd <= 7
    uint8_t op =      (uint8_t) ((instruction & 0b0001100000000000) >> 11);
    uint8_t offset5 = (uint8_t) ((instruction & 0b0000011111000000) >> 6);
    uint8_t rs =      (uint8_t) ((instruction & 0b0000000000111000) >> 3);
    uint8_t rd =      (uint8_t)  (instruction & 0b0000000000000111);

    if (op == 0) {
        // LSL Rd, Rs, #Offset5
        // Rd := Rs << Offset5
        // Safe because rs and rd are no higher than 7
        printf("LSL R%u, R%u, #%u\n", rd, rs, offset5);
        vm->registers[rd] = (vm->registers[rs]) << offset5;
        compareSetNZ(vm, vm->registers[rd]);

        // The carry flag is special in this case - have we shifted any ones off the left hand side?
        // We can find out if that's the case by inspecting the left `offset5` bits
        // And we can do that by starting with all ones and shifting to the left by `offset5` then ANDing that
        // with Rs
        if (offset5 != 0) {
            // Carry is only changed if the offset was nonzero
            uint32_t mask = 0xFFFFFFFF << offset5;
            if ((vm->registers[rs]) & mask) {
                // We did shift off some ones
                vm_set_cpsr_c(vm);
            } else {
                vm_clr_cpsr_c(vm);
            }
        }
    } else if (op == 1) {
        // LSR Rd, Rs, #Offset5
        // Rd := Rs >>(logical) Offset5
        // Safe because rd and rs and no higher than 7
        printf("LSR R%u, R%u, #%u\n", rd, rs, offset5);
        vm->registers[rd] = (vm->registers[rs]) >> offset5;
        compareSetNZ(vm, vm->registers[rd]);

        // The carry flag here is sensible, although it's not really a "carry"
        // If we shift any ones off the right hand side, then set C; if not, clear it
        // Carry is set even if shift was 0, although in that case it will always be false
        uint32_t mask = 0xFFFFFFFF >> (32 - offset5);
        if ((vm->registers[rs]) & mask) {
            // We did shift off some ones
            vm_set_cpsr_c(vm);
        } else {
            vm_clr_cpsr_c(vm);
        }
    } else if (op == 2) {
        // ASR Rd, Rs, #Offset5
        // Rd := Rs >>(arithmetic) Offset5
        // Safe because rd and rs are no higher than 7
        // In C, while technically this is implementation-defined, in general we do an arithmetic shift using signed int
        printf("ASR R%u, R%u, #%u\n", rd, rs, offset5);
        vm->registers[rd] = (uint32_t) (((int32_t) (vm->registers[rs])) >> offset5);
        compareSetNZ(vm, vm->registers[rd]);

        // The carry flag here is sensible, although it's not really a "carry"
        // If we shift any ones off the right hand side, then set C; if not, clear it
        // Carry is set even if shift was 0, although in that case it will always be false
        uint32_t mask = 0xFFFFFFFF >> (32 - offset5);
        if ((vm->registers[rs]) & mask) {
            // We did shift off some ones
            vm_set_cpsr_c(vm);
        } else {
            vm_clr_cpsr_c(vm);
        }
    } else {
        // We should never get here
    }
}


/**
 * Add/subtract
 * Documented as instruction 2 in the manual
 * Covers instructions with first byte 00011XXX
 * @param vm
 * @param instruction
 */
void tliAddSubtract(VM_instance* vm, uint16_t instruction)
{
    // Decode the operation
    // 0 <= i <= 1
    // 0 <= op <= 1
    // 0 <= rn <= 7
    // 0 <= rs <= 7
    // 0 <= rd <= 7
    uint8_t i =  (uint8_t) ((instruction & 0b0000010000000000) >> 10);
    uint8_t op = (uint8_t) ((instruction & 0b0000001000000000) >> 9);
    uint8_t rn = (uint8_t) ((instruction & 0b0000000111000000) >> 6);
    uint8_t rs = (uint8_t) ((instruction & 0b0000000000111000) >> 3);
    uint8_t rd = (uint8_t)  (instruction & 0b0000000000000111);

    if (op == 0) {
        if (i == 0) {
            // ADD Rd, Rs, Rn
            // Rd := Rn + Rs
            // Safe because rd, rn and rs are no higher than 7
            printf("ADD r%u, r%u, r%u\n", rd, rs, rn);
            vm->registers[rd] = vm->registers[rn] + vm->registers[rs];
            compareSetNZ(vm, vm->registers[rd]);
            compareSetCV(vm, vm->registers[rn], vm->registers[rs]);
        } else {
            // ADD Rd, Rs, #Offset3
            // Rd := Rs + Offset3
            // Safe because rd, rs are no higher than 7
            // rn is equivalent to Offset3 (same bits used in encoding)
            printf("ADD r%u, r%u, #%u\n", rd, rs, rn);
            vm->registers[rd] = vm->registers[rs] + rn;
            compareSetNZ(vm, vm->registers[rd]);
            compareSetCV(vm, vm->registers[rs], rn);
        }
    } else {
        if (i == 0) {
            // SUB Rd, Rs, Rn
            // Rd := Rn - Rs
            // Safe because rd, rn and rs are no higher than 7
            printf("SUB r%u, r%u, r%u\n", rd, rs, rn);
            vm->registers[rd] = vm->registers[rn] - vm->registers[rs];
            compareSetNZ(vm, vm->registers[rd]);
            compareSetCV(vm, vm->registers[rn], 0 - vm->registers[rs]);
        } else {
            // SUB Rd, Rs, #Offset3
            // Rd := Rs - Offset3
            // Safe because rd, rs are no higher than 7
            // rn is equivalent to Offset3 (same bits used in encoding)
            printf("SUB r%u, r%u, #%u\n", rd, rs, rn);
            vm->registers[rd] = vm->registers[rs] - rn;
            compareSetNZ(vm, vm->registers[rd]);
            compareSetCV(vm, vm->registers[rs], 0 - rn);
        }
    }
}


/**
 * Move/compare/add/subtract immediate.
 * Documented as instruction 3 in manual.
 * Covers instructions with first byte 001XXXXX
 * @param vm
 * @param instruction
 */
void tliMovCmpAddSubImmediate(VM_instance* vm, uint16_t instruction)
{
    // Decode the operation
    // 0 <= op <= 3
    // 0 <= rd <= 7
    // 0 <= offset <= 255
    uint8_t op =     (uint8_t) ((instruction & 0b0001100000000000) >> 11);
    uint8_t rd =     (uint8_t) ((instruction & 0b0000011100000000) >> 8);
    uint8_t offset = (uint8_t)  (instruction & 0b0000000011111111);

    if (op == 0b00) {
        // MOV Rd, #Offset
        // Moves the 8-bit immediate value `Offset` into Rd
        printf("MOV r%u, #%u\n", rd, offset);
        vm->registers[rd] = offset;
        compareSetNZ(vm, offset);
    } else if (op == 0b01) {
        // CMP Rd, #Offset
        // Compares the register with the 8-bit offset
        printf("CMP r%u, #%u\n", rd, offset);

        // Set comparison registers based on Rd - Offset
        compareSetNZ(vm, vm->registers[rd] - offset);
        compareSetCV(vm, vm->registers[rd], 0 - offset);
    } else if (op == 0b10) {
        // ADD Rd, #Offset
        // Adds the offset to the register
        printf("ADD r%u, #%u\n", rd, offset);
        compareSetCV(vm, vm->registers[rd], offset);
        vm->registers[rd] += offset;
        compareSetNZ(vm, vm->registers[rd]);
    } else {
        // SUB Rd, #Offset
        // Subtracts the offset from the register
        printf("SUB r%u, #%u\n", rd, offset);
        compareSetCV(vm, vm->registers[rd], 0L - offset);
        vm->registers[rd] -= offset;
        compareSetNZ(vm, vm->registers[rd]);
    }
}


/**
 * ALU operations
 * Documented as instruction 4 in the manual
 * Covers instructions with first byte 010000XX
 * @param vm
 * @param instruction
 */
void tliALUOperations(VM_instance* vm, uint16_t instruction)
{
    // Decode the operation
    uint8_t op = (instruction & 0b0000001111000000) >> 6;
    uint8_t rs = (instruction & 0b0000000000111000) >> 3;
    uint8_t rd = (instruction & 0b0000000000000111);

    if (op == 0b0000) {
        // AND Rd, Rs
        printf("AND r%u, r%u\n", rd, rs);
        vm->registers[rd] = vm->registers[rd] & vm->registers[rs];
        compareSetNZ(vm, vm->registers[rd]);
    } else if (op == 0b0001) {
        // EOR Rd, Rs
        printf("EOR r%u, r%u\n", rd, rs);
        vm->registers[rd] = vm->registers[rd] ^ vm->registers[rs];
        compareSetNZ(vm, vm->registers[rd]);
    } else if (op == 0b0010) {
        // LSL Rd, Rs
        printf("LSL r%u, r%u\n", rd, rs);
        if ((vm->registers[rs] & 0xFF) != 0) {
            // Carry is only changed if the offset was nonzero
            uint32_t mask = 0xFFFFFFFF << vm->registers[rs];
            if ((vm->registers[rd]) & mask) {
                // We did shift off some ones
                vm_set_cpsr_c(vm);
            } else {
                vm_clr_cpsr_c(vm);
            }
        }
        vm->registers[rd] = vm->registers[rd] << vm->registers[rs];
        compareSetNZ(vm, vm->registers[rd]);
    } else if (op == 0b0011) {
        // LSR Rd, Rs
        printf("LSR r%u, r%u\n", rd, rs);
        uint32_t mask = 0xFFFFFFFF >> (32 - vm->registers[rs]);
        if ((vm->registers[rd]) & mask) {
            // We did shift off some ones
            vm_set_cpsr_c(vm);
        } else {
            vm_clr_cpsr_c(vm);
        }
        vm->registers[rd] = vm->registers[rd] >> vm->registers[rs];
        compareSetNZ(vm, vm->registers[rd]);
    } else if (op == 0b0100) {
        // ASR Rd, Rs
        printf("ASR r%u, r%u\n", rd, rs);
        uint32_t mask = 0xFFFFFFFF >> (32 - vm->registers[rs]);
        if ((vm->registers[rs] & 0xFF) != 0) {
            if ((vm->registers[rd]) & mask) {
                // We did shift off some ones
                vm_set_cpsr_c(vm);
            } else {
                vm_clr_cpsr_c(vm);
            }
        }
        vm->registers[rd] = (uint32_t) (((int32_t) (vm->registers[rd])) >> vm->registers[rs]);
        compareSetNZ(vm, vm->registers[rd]);
    } else if (op == 0b0101) {
        // ADC Rd, Rs
        printf("ADC r%u, r%u\n", rd, rs);

        uint8_t carryAtStart = vm_get_cpsr_c(vm);

        // Calculating the carry and overflow in 2 stages
        compareSetCV(vm, vm->registers[rd], vm->registers[rs]);
        uint8_t carry1 = vm_get_cpsr_c(vm);
        uint8_t overflow1 = vm_get_cpsr_v(vm);
        compareSetCV(vm, vm->registers[rd] + vm->registers[rs], 1);
        uint8_t carry2 = vm_get_cpsr_c(vm);
        uint8_t overflow2 = vm_get_cpsr_v(vm);

        if (carry1 || carry2) {
            vm_set_cpsr_c(vm);
        } else {
            vm_clr_cpsr_c(vm);
        }
        if (overflow1 || overflow2) {
            vm_set_cpsr_v(vm);
        } else {
            vm_clr_cpsr_v(vm);
        }

        vm->registers[rd] = vm->registers[rd] + vm->registers[rs] + carryAtStart;
        compareSetNZ(vm, vm->registers[rd]);
    } else if (op == 0b0110) {
        // SBC Rd, Rs
        printf("SBC r%u, r%u\n", rd, rs);

        uint8_t carryAtStart = vm_get_cpsr_c(vm);

        // Calculating the carry and overflow in 2 stages
        compareSetCV(vm, vm->registers[rd], 0UL - vm->registers[rs]);
        uint8_t carry1 = vm_get_cpsr_c(vm);
        uint8_t overflow1 = vm_get_cpsr_v(vm);
        compareSetCV(vm, vm->registers[rd] - vm->registers[rs], !carryAtStart);
        uint8_t carry2 = vm_get_cpsr_c(vm);
        uint8_t overflow2 = vm_get_cpsr_v(vm);

        if (carry1 || carry2) {
            vm_set_cpsr_c(vm);
        } else {
            vm_clr_cpsr_c(vm);
        }
        if (overflow1 || overflow2) {
            vm_set_cpsr_v(vm);
        } else {
            vm_clr_cpsr_v(vm);
        }

        vm->registers[rd] = vm->registers[rd] - vm->registers[rs] + !carryAtStart;
        compareSetNZ(vm, vm->registers[rd]);
    } else if (op == 0b0111) {
        // ROR Rd, Rs
        printf("ROR r%u, r%u\n", rd, rs);

        vm->registers[rd] = (vm->registers[rd] >> vm->registers[rs]) | (vm->registers[rd] << (32 - vm->registers[rs]));
        compareSetNZ(vm, vm->registers[rd]);
        if ((vm->registers[rs] & 0xFF) != 0) {
            // Carry flag only affected if lower 8 bits of Rs is non-zero
            if (~(0xFFFFFFFF << (vm->registers[rs])) & (vm->registers[rd])) {
                // Carry should be set if the lowest Rs bits of Rd aren't all zero
                vm_set_cpsr_c(vm);
            } else {
                vm_clr_cpsr_c(vm);
            }
        }
    } else if (op == 0b1000) {
        // TST Rd, Rs
        printf("TST r%u, r%u\n", rd, rs);

        // Just set condition bits without changing rd
        compareSetNZ(vm, (vm->registers[rd]) & (vm->registers[rs]));
    } else if (op == 0b1001) {
        // NEG Rd, Rs
        printf("NEG r%u, r%u\n", rd, rs);

        compareSetNZ(vm, 0 - vm->registers[rs]);
        compareSetCV(vm, 0, -vm->registers[rs]);
        vm->registers[rd] = 0 - vm->registers[rs];
    } else if (op == 0b1010) {
        // CMP Rd, Rs
        printf("CMP r%u, r%u\n", rd, rs);

        // Compare based on the result of rd - rs
        compareSetNZ(vm, vm->registers[rd] - vm->registers[rs]);
        compareSetCV(vm, vm->registers[rd], 0 - vm->registers[rs]);
    } else if (op == 0b1011) {
        // CMN Rd, Rs
        printf("CMN r%u, r%u\n", rd, rs);

        // Compare based on the result of rd + rs
        compareSetNZ(vm, vm->registers[rd] + vm->registers[rs]);
        compareSetCV(vm, vm->registers[rd], vm->registers[rs]);
    } else if (op == 0b1100) {
        // ORR Rd, Rs
        printf("ORR r%u, r%u\n", rd, rs);

        vm->registers[rd] = (vm->registers[rd]) | (vm->registers[rs]);
        compareSetNZ(vm, vm->registers[rd]);
    } else if (op == 0b1101) {
        // MUL Rd, Rs
        printf("MUL r%u, r%u\n", rd, rs);

        vm->registers[rd] = vm->registers[rd] * vm->registers[rs];
        compareSetNZ(vm, vm->registers[rd]);
    } else if (op == 0b1110) {
        // BIC Rd, Rs
        printf("BIC r%u, r%u\n", rd, rs);

        vm->registers[rd] = (vm->registers[rd]) & (~(vm->registers[rs]));
        compareSetNZ(vm, vm->registers[rd]);
    } else if (op == 0b1111) {
        // MVN Rd, Rs
        printf("MVN r%u, r%u\n", rd, rs);

        vm->registers[rd] = ~(vm->registers[rs]);
        compareSetNZ(vm, vm->registers[rd]);
    }
}


/**
 * High register operations/branch exchange
 * Documented as instruction 5 in the manual
 * Coers instructions with first byte 010001XX
 * @param vm
 * @param instruction
 */
void tliHighRegOperations(VM_instance* vm, uint16_t instruction)
{
    // Decode the pieces of the operation
    // 0 <= op <= 3
    // 0 <= h1_and_2 <= 3
    // 0 <= rs <= 7
    // 0 <= rd <= 7
    uint8_t op =       (uint8_t) ((instruction & 0b0000001100000000) >> 8);
    uint8_t h1_and_2 = (uint8_t) ((instruction & 0b0000000011000000) >> 6);
    uint8_t rs =       (uint8_t) ((instruction & 0b0000000000111000) >> 3);
    uint8_t rd =       (uint8_t)  (instruction & 0b0000000000000111);

    if (op == 0b00) {
        if (h1_and_2 == 0b01) {
            // ADD Rd, Hs
            // Sum together the values in a low register Rd and a high register Rs (8+Rs),
            // and store the result in low register Rd
            printf("ADD r%u, H%u\n", rd, 8+rs);
            compareSetNZ(vm, vm->registers[rd] + vm->registers[8+rs]);
            compareSetCV(vm, vm->registers[rd], vm->registers[8+rs]);
            vm->registers[rd] = vm->registers[rd] + vm->registers[8+rs];
        } else if (h1_and_2 == 0x10) {
            // ADD Hd, Rs
            // Sum together the values in high register Hd (8+Rd) and low register Rs
            // then store the result in Hd
            printf("ADD H%u, R%u\n", 8+rd, rs);
            compareSetNZ(vm, vm->registers[8+rd] + vm->registers[rs]);
            compareSetCV(vm, vm->registers[8+rd], vm->registers[rs]);
            vm->registers[8+rd] = vm->registers[rs];
        } else if (h1_and_2 == 0b11) {
            // ADD Hd, Hs
            // Sum together the values in the high registers Hd (8+Rd) and Hs (8+Rs)
            // then store the result in Hd
            printf("ADD H%u, H%u\n", 8+rd, 8+rs);
            compareSetNZ(vm, vm->registers[8+rd] + vm->registers[8+rs]);
            compareSetCV(vm, vm->registers[8+rd], vm->registers[8+rs]);
            vm->registers[8+rd] = vm->registers[8+rd] + vm->registers[8+rs];
        } else {
            printf("Invalid command %x\n", instruction);
            vm->finished = true;
        }
    } else if (op == 0b01) {
        if (h1_and_2 == 0b01) {
            // CMP Rd, Hs
            printf("CMP r%u, h%u\n", rd, 8+rs);

            compareSetNZ(vm, vm->registers[rd] - vm->registers[8+rs]);
            compareSetCV(vm, vm->registers[rd], 0 - vm->registers[8+rs]);
        } else if (h1_and_2 == 0b10) {
            // CMP Hd, Rs
            printf("CMP h%u, r%u\n", 8+rd, rs);

            compareSetNZ(vm, vm->registers[8+rd] - vm->registers[rs]);
            compareSetCV(vm, vm->registers[8+rd], 0 - vm->registers[rs]);
        } else if (h1_and_2 == 0b11) {
            // CMP Hd, Hs
            printf("CMP h%u, h%u\n", 8+rd, 8+rs);
            compareSetNZ(vm, vm->registers[8+rd] - vm->registers[8+rs]);
            compareSetCV(vm, vm->registers[8+rd], vm->registers[8+rs]);
        } else {
            printf("Invalid command %x\n", instruction);
            vm->finished = true;
        }
    } else if (op == 0b10) {
        if (h1_and_2 == 0x01) {
            // MOV Rd, Hs
            // Moves a value from high register Hs (8+Rs) into low register Rd
            // rd <= 7 and (8+rs) <= 15, so this is safe
            printf("MOV r%u, r%u\n", rd, 8+rs);
            vm->registers[rd] = vm->registers[8+rs];
        } else if (h1_and_2 == 0x10) {
            // MOV Hd, Rs
            printf("MOV h%u, r%u\n", 8+rd, rs);
            vm->registers[8+rd] = vm->registers[rs];
        } else if (h1_and_2 == 0x11) {
            // MOV Hd, Hs
            printf("MOV h%u, h%u\n", 8+rd, 8+rs);
            vm->registers[8+rd] = vm->registers[8+rs];
        } else {
            printf("Invalid command %x\n", instruction);
            vm->finished = true;
        }
    } else {
        if (h1_and_2 == 0b00) {
            // BX Rs
            printf("BX r%u\n", rs);
            vm_program_counter(vm) = rs & 0xFFFFFFFE;
        } else if (h1_and_2 == 0b01) {
            // BX Hs
            printf("BX h%u\n", 8+rs);
            vm_program_counter(vm) = rs & 0xFFFFFFFE;
        } else {
            printf("Invalid command %x\n", instruction);
            vm->finished = true;
        }
    }
}


/**
 * PC-relative load
 * Documented as instruction 6 in the manual
 * Covers instructions with first byte 01001XXX
 * @param vm
 * @param instruction
 */
void tliPCRelativeLoad(VM_instance* vm, uint16_t instruction)
{
    // Find the destination register and the (shifted) offset
    uint8_t rd =   (instruction & 0b0000011100000000) >> 8;
    uint8_t word8 = instruction & 0b0000000011111111;

    // Calculate the offset by multiplying word8 by 4
    uint16_t offset = word8 << 2;

    printf("LDR r%u, [PC, #%u]\n", rd, offset);

    // Calculate the memory location by adding the offset to the PC
    uint32_t addr = (vm_program_counter(vm) & 0xFFFFFFFC) + offset;

    // Load a word into the destination register (big endian)
    vm->registers[rd] = load(vm, addr, 4);
}


/**
 * Load/store with register offset
 * Documented as instruction 7 in the manual
 * Covers instructions with first byte 0101XX0X
 * @param vm
 * @param instruction
 */
void tliLoadWithRegOffset(VM_instance* vm, uint16_t instruction)
{
    uint8_t load_or_store = (instruction & 0b0000100000000000) >> 11;
    uint8_t byte_or_word =  (instruction & 0b0000010000000000) >> 10;
    uint8_t ro =            (instruction & 0b0000000111000000) >> 6;
    uint8_t rb =            (instruction & 0b0000000000111000) >> 3;
    uint8_t rd =            (instruction & 0b0000000000000111);

    // The address for all instructions is Rb + Ro
    uint32_t addr = vm->registers[rb] + vm->registers[ro];

    if (load_or_store == 0) {
        if (byte_or_word == 0) {
            // STR Rd, [Rb, Ro]
            // Store a word from Rd into the address
            printf("STR r%u, [r%u, r%u]\n", rd, rb, ro);
            store(vm, addr, vm->registers[rd], 4);
        } else {
            // STRB Rd, [Rb, Ro]
            printf("STRB r%u, [r%u, r%u]\n", rd, rb, ro);
            store(vm, addr, vm->registers[rd] & 0xFF, 1);
        }
    } else {
        if (byte_or_word == 0) {
            // LDR Rd, [Rb, Ro]
            // Load a word from Rb + Ro into Rd
            printf("LDR r%u, [r%u, r%u]\n", rd, rb, ro);
            vm->registers[rd] = load(vm, addr, 4);
        } else {
            // LDRB Rd, [Rb, Ro]
            // Load a byte from Rb + Ro into Rd
            printf("LDRB r%u, [r%u, r%u]\n", rd, rb, ro);
            vm->registers[rd] = load(vm, addr, 1);
        }
    }
}


/**
 * Load/store sign-extended byte/halfword
 * Documented as instruction 8 in the manual
 * Covers instructions with first byte 0101XX1X
 * @param vm
 * @param instruction
 */
void tliLoadStoreSignExtendedByte(VM_instance* vm, uint16_t instruction)
{
    uint8_t h =       (instruction & 0b0000100000000000) >> 11;
    uint8_t sgn_ext = (instruction & 0b0000010000000000) >> 10;
    uint8_t ro =      (instruction & 0b0000000111000000) >> 6;
    uint8_t rb =      (instruction & 0b0000000000111000) >> 3;
    uint8_t rd =      (instruction & 0b0000000000000111);

    // The address for all instructions is Rb + Ro
    uint32_t addr = vm->registers[rb] + vm->registers[ro];

    if (sgn_ext == 0) {
        if (h == 0) {
            // STRH Rd, [Rb, Ro]
            // Store half-word from Rd into memory address Rb+Ro
            printf("STRH r%u, [r%u, r%u]\n", rd, rb, ro);
            store(vm, addr, vm->registers[rd] & 0x0000FFFF, 2);
        } else {
            // LDRH Rd, [Rb, Ro]
            // Load half-word from Rb+Ro into register Rd
            printf("LDRH r%u, [r%u, r%u]\n", rd, rb, ro);
            vm->registers[rd] = load(vm, addr, 2);
        }
    } else {
        if (h == 0) {
            // LDSB Rd, [Rb, Ro]
            // Load byte from Rb+Ro, sign-extend it to word size, and put it in Rd
            printf("LDSB r%u, [r%u, r%u]\n", rd, rb, ro);
            uint32_t value = load(vm, addr, 1);
            value = (value & 0x000000FF) | ((value & 0x80) ? 0xFFFFFF00 : 0);
            vm->registers[rd] = value;
        } else {
            // LDSH Rd, [Rb, Ro]
            // Load half-word from Rb+Ro, sign-extend it to word size, and put it in Rd
            printf("LDSH r%u, [r%u, r%u]\n", rd, rb, ro);
            uint32_t value = load(vm, addr, 2);
            value = (value & 0x0000FFFF) | ((value & 0x8000) ? 0xFFFF0000 : 0);
            vm->registers[rd] = value;
        }
    }
}


/**
 * Load/store with immediate offset
 * Documented as instruction 9 in the manual
 * Covers instructions with first byte 011XXXXX
 * @param vm
 * @param instruction
 */
void tliLoadStoreWithImmediateOffset(VM_instance* vm, uint16_t instruction)
{
    uint8_t byte_or_word =  (instruction & 0b0001000000000000) >> 12;
    uint8_t load_or_store = (instruction & 0b0000100000000000) >> 11;
    uint8_t offset5 =       (instruction & 0b0000011111000000) >> 6;
    uint8_t rb =            (instruction & 0b0000000000111000) >> 3;
    uint8_t rd =            (instruction & 0b0000000000000111);

    if (byte_or_word == 0) {
        uint32_t addr = vm->registers[rb] + (offset5 << 2);
        if (load_or_store == 0) {
            // STR Rd, [Rb, #lmm]
            // Store the contents of Rd into the word starting at memory address Rb+lmm
            printf("STR r%u, [r%u, #%u]\n", rd, rb, (offset5 << 2));
            store(vm, addr, vm->registers[rd], 4);
        } else {
            // LDR Rd, [Rb, #lmm]
            // Load the word at Rb+lmm into register Rd
            printf("LDR r%u, [r%u, #%u]\n", rd, rb, (offset5 << 2));
            vm->registers[rd] = load(vm, addr, 4);
        }
    } else {
        uint32_t addr = vm->registers[rb] + offset5;
        if (load_or_store == 0) {
            // STRB Rd, [Rb, #lmm]
            // Store the least significant byte of Rd in memory address Rb+lmm
            printf("STRB r%u, [r%u, #%u]\n", rd, rb, offset5);
            store(vm, addr, vm->registers[rd], 1);
        } else {
            // LDRB Rd, [Rb, #lmm]
            // Load the byte at Rb+lmm into register Rd
            printf("LDRB r%u, [r%u, #%u]\n", rd, rb, offset5);
            vm->registers[rd] = load(vm, addr, 1);
        }
    }
}


/**
 * Load/store halfword
 * Documented as instruction 10 in the manual
 * Covers instructions with first byte 1000XXXX
 * @param vm
 * @param instruction
 */
void tliLoadStoreHalfWord(VM_instance* vm, uint16_t instruction)
{
    uint8_t load_or_store = (instruction & 0b0000100000000000) >> 11;
    uint8_t offset5 =       (instruction & 0b0000011111000000) >> 6;
    uint8_t rb =            (instruction & 0b0000000000111000) >> 3;
    uint8_t rd =            (instruction & 0b0000000000000111);

    uint32_t addr = vm->registers[rb] + (offset5 << 1);

    if (load_or_store == 0) {
        // STRH Rd, [Rb, #lmm]
        // Stores the least significant 16 bits of Rd into memory address Rb+lmm
        printf("STRH r%u, [r%u, #%u]\n", rd, rb, (offset5 << 1));
        store(vm, addr, vm->registers[rd], 2);
    } else {
        // LDRH Rd, [Rb, #lmm]
        // Loads the half-word from address Rb+lmm and stores it in register Rd
        printf("LDRH r%u, [r%u, #%u]\n", rd, rb, (offset5 << 1));
        vm->registers[rd] = load(vm, addr, 2);
    }
}


/**
 * SP-relative load/store
 * Documented as instruction 11 in the manual
 * Covers instructions with first byte 1001XXXX
 * @param vm
 * @param instruction
 */
void tliSPRelativeLoad(VM_instance* vm, uint16_t instruction)
{
    uint8_t load_or_store = (instruction & 0b0000100000000000) >> 11;
    uint8_t rd =            (instruction & 0b0000011100000000) >> 8;
    uint8_t word8 =         (instruction & 0b0000000011111111);

    uint32_t addr = vm_stack_pointer(vm) + ((uint32_t) word8 << 2);

    if (load_or_store == 0) {
        // STR Rd, [SP, #lmm]
        // Store the contents of register Rd into address SP+lmm
        printf("STR r%u, [SP, #%u]\n", rd, ((uint16_t) word8) << 2);
        store(vm, addr, vm->registers[rd], 4);
    } else {
        // LDR Rd, [SP, #lmm]
        // Load the contents of address SP+lmm into register Rd
        printf("LDR r%u, [SP, #%u]\n", rd, ((uint16_t) word8) << 2);
        vm->registers[rd] = load(vm, addr, 4);
    }
}


/**
 * Load address
 * Documented as instruction 12 in the manual
 * Covers instructions with first byte 1010XXXX
 * @param vm
 * @param instruction
 */
void tliLoadAddress(VM_instance* vm, uint16_t instruction)
{
    uint8_t sp =    (instruction & 0b0000100000000000) >> 11;
    uint8_t rd =    (instruction & 0b0000011100000000) >> 8;
    uint8_t word8 = (instruction & 0b0000000011111111);

    uint16_t lmm = ((uint16_t) word8) << 2;

    if (sp == 0) {
        // ADD Rd, PC, #lmm
        // Add lmm to PC and store the resulting address in Rd
        printf("ADD r%u, PC, #%u\n", rd, lmm);
        vm->registers[rd] = vm_program_counter(vm) + lmm;
    } else {
        // ADD Rd, SP, #lmm
        // Add lmm to SP and store the resulting address in Rd
        printf("ADD r%u, SP, #%u\n", rd, lmm);
        vm->registers[rd] = vm_stack_pointer(vm) + lmm;
    }
}


/**
 * Add offset to stack pointer
 * Documented as instruction 13 in the manual
 * Covers instructions with first byte 10110000
 * @param vm
 * @param instruction
 */
void tliAddOffsetToSP(VM_instance* vm, uint16_t instruction)
{
    uint8_t sign =   (instruction & 0b0000000010000000) >> 7;
    uint8_t sword7 = (instruction & 0b0000000001111111);

    uint16_t lmm = ((uint16_t) sword7) << 2;

    if (sign == 0) {
        // ADD SP, #lmm
        // Increase the stack pointer by lmm
        printf("ADD SP, #%u\n", lmm);
        vm_stack_pointer(vm) = vm_stack_pointer(vm) + lmm;
    } else {
        // ADD SP, #-lmm
        // Decrease the stack pointer by lmm
        printf("ADD SP, #-%u\n", lmm);
        vm_stack_pointer(vm) = vm_stack_pointer(vm) - lmm;
    }
}


/**
 * Push/pop registers
 * Documented as instruction 14 in the manual
 * Covers instructions with first byte 1011X10X
 * @param vm
 * @param instruction
 */
void tliPushPopRegisters(VM_instance* vm, uint16_t instruction)
{
    uint8_t load_or_store = (instruction & 0b0000100000000000) >> 10;
    uint8_t pc_lr =      (instruction & 0b0000000100000000) >> 8;
    uint8_t rlist =         (instruction & 0b0000000011111111);

    // Calculate which registers are included in the register list
    uint8_t numRegistersInvolved = 0;
    bool use_registers[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    for (uint8_t i = 0; i < 8; i++) {
        if (rlist & (1 << (7-i))) {
            use_registers[i] = 1;
            ++numRegistersInvolved;
        }
    }

    if ((load_or_store == 0) && (pc_lr == 1)) {
        // If this is PUSH {rlist, LR} then use LR
        use_registers[14] = 1;
        ++numRegistersInvolved;
    } else if ((load_or_store == 1) && (pc_lr == 1)) {
        // If this is POP {rlist, PC} then use PC
        use_registers[15] = 1;
        ++numRegistersInvolved;
    }

    // Now actually do it
    // Pushing goes from the highest register to the lowest, popping goes from lowest to highest
    if (load_or_store == 0) {
        // Push
        printf("push {...*%u}\n", numRegistersInvolved);
        for (int8_t i = 15; i >= 0; i--) {
            if (use_registers[i]) {
                vm_stack_pointer(vm) -= 4;
                store(vm, vm_stack_pointer(vm), vm->registers[i], 4);
            }
        }
    } else {
        // Pop
        printf("pop {...*%u}\n", numRegistersInvolved);
        for (int8_t i = 0; i < 16; i++) {
            if (use_registers[i]) {
                vm->registers[i] = load(vm, vm_stack_pointer(vm), 4);
                vm_stack_pointer(vm) += 4;
            }
        }
    }
}


/**
 * Multiple load/store
 * Documented as instruction 15 in the manual
 * Covers instructions with first byte 1100XXXX
 * @param vm
 * @param instruction
 */
void tliMultipleLoadStore(VM_instance* vm, uint16_t instruction)
{
    uint8_t load_or_store = (instruction & 0b0000100000000000) >> 11;
    uint8_t rb =            (instruction & 0b0000011100000000) >> 8;
    uint8_t rlist =         (instruction & 0b0000000011111111);

    uint32_t baseAddress = vm->registers[rb];

    // Calculate which registers are included in the register list
    uint8_t numRegistersInvolved = 0;
    bool use_registers[16] = {0, 0, 0, 0, 0, 0, 0, 0};
    for (uint8_t i = 0; i < 8; i++) {
        if (rlist & (1 << (7-i))) {
            use_registers[i] = 1;
            ++numRegistersInvolved;
        }
    }

    // Perform the operation
    if (load_or_store == 0) {
        // STMIA Rb!, {rlist}
        // Store the registers in rlist starting at base address rb
        printf("STMIA r%u!, {...*%u}\n", rb, numRegistersInvolved);
        for (int8_t i = 0; i < 8; i++) {
            if (use_registers[i]) {
                store(vm, baseAddress, vm->registers[i], 4);
                baseAddress += 4;
            }
        }
    } else {
        // LDMIA Rb!, {rlist}
        // Load the registers in rlist starting at base address rb
        printf("LDMIA r%u!, {...*%u}\n", rb, numRegistersInvolved);
        for (uint8_t i = 0; i < 8; i++) {
            if (use_registers[i]) {
                vm->registers[i] = load(vm, baseAddress, 4);
                baseAddress += 4;
            }
        }
    }

    // Save the address back
    vm->registers[rb] = baseAddress;
}


/**
 * Conditional branch
 * Documented as instruction 16 in the manual
 * Covers instrutions with first byte 1101XXXX, except 11011111
 * @param vm
 * @param instruction
 */
void tliConditionalBranch(VM_instance* vm, uint16_t instruction)
{
    uint8_t cond =    (instruction & 0b0000111100000000) >> 8;
    int8_t soffset8 = (int8_t) (instruction & 0b0000000011111111);

    uint32_t targetAddress = vm_program_counter(vm) + soffset8;

    bool condition;
    char* printFormat;
    if (cond == 0b0000) {
        // BEQ label
        // Branch if Z set (equal)
        printFormat = "BEQ %u\n";
        condition = vm_get_cpsr_z(vm);
    } else if (cond == 0b0001) {
        // BNE label
        // Branch if Z clear (not equal)
        printFormat = "BNE %u\n";
        condition = !vm_get_cpsr_z(vm);
    } else if (cond == 0b0010) {
        // BCS label
        // Branch if C set (unsigned higher or same)
        printFormat = "BCS %u\n";
        condition = vm_get_cpsr_c(vm);
    } else if (cond == 0b0011) {
        // BCC label
        // Branch if C clear (unsigned lower)
        printFormat = "BCC %u\n";
        condition = !vm_get_cpsr_c(vm);
    } else if (cond == 0b0100) {
        // BMI label
        // Branch if N set (negative)
        printFormat = "BMI %u\n";
        condition = vm_get_cpsr_n(vm);
    } else if (cond == 0b0101) {
        // BPL label
        // Branch if N clear (positive or zero)
        printFormat = "BPL %u\n";
        condition = !vm_get_cpsr_n(vm);
    } else if (cond == 0b0110) {
        // BVS label
        // Branch if V set (overflow)
        printFormat = "BVS %u\n";
        condition = vm_get_cpsr_v(vm);
    } else if (cond == 0b0111) {
        // BVC label
        // Branch if V clear (no overflow)
        printFormat = "BVC %u\n";
        condition = !vm_get_cpsr_v(vm);
    } else if (cond == 0b1000) {
        // BHI label
        // Branch if C set and Z clear (unsigned higher)
        printFormat = "BHI %u\n";
        condition = vm_get_cpsr_c(vm) && !vm_get_cpsr_z(vm);
    } else if (cond == 0b1001) {
        // BLS label
        // Branch if C clear or Z set (unsigned lower or same)
        printFormat = "BLS %u\n";
        condition = !vm_get_cpsr_c(vm) || vm_get_cpsr_z(vm);
    } else if (cond == 0b1010) {
        // BGE label
        // Branch if N set and V set, or N clear and V clear (greater or equal)
        printFormat = "BGE %u\n";
        condition = (vm_get_cpsr_n(vm) && vm_get_cpsr_v(vm)) || (!vm_get_cpsr_n(vm) || !vm_get_cpsr_v(vm));
    } else if (cond == 0b1011) {
        // BLT label
        // Branch if N set and V clear, or N clear and V set (less than)
        printFormat = "BLT %u\n";
        condition = (vm_get_cpsr_n(vm) && !vm_get_cpsr_v(vm)) || (!vm_get_cpsr_n(vm) && vm_get_cpsr_v(vm));
    } else if (cond == 0b1100) {
        // BGT label
        // Branch if Z clear, and either N set and V set or N clear and V clear (greater than)
        printFormat = "BGT %u\n";
        condition = !vm_get_cpsr_z(vm) && ((vm_get_cpsr_n(vm) && vm_get_cpsr_v(vm)) || (!vm_get_cpsr_n(vm) && !vm_get_cpsr_v(vm)));
    } else if (cond == 0b1101) {
        // BLE label
        // Branch if Z set, or N set and V clear, or N clear and V set (less than or equal)
        printFormat = "BLE %u\n";
        condition = vm_get_cpsr_z(vm) || (vm_get_cpsr_n(vm) && !vm_get_cpsr_v(vm)) || (!vm_get_cpsr_n(vm) && vm_get_cpsr_v(vm));
    } else {
        printf("Invalid command %x", instruction);
        vm->finished = true;
    }
    printf(printFormat, targetAddress);
}


/**
 * Software interrupt
 * Documented as instruction 17 in manual
 * Covers instructions with first byte 11011111
 * @param vm
 * @param instruction
 */
void tliSoftwareInterrupt(VM_instance* vm, uint16_t instruction)
{
    // Decode the instruction
    uint8_t value = instruction & 0x00FF;
    printf("SWI #%u\n", value);

    // Move the address of the next instruction into the link register
    vm_link_register(vm) = vm_program_counter(vm);

    // Trigger the interrupt
    vm->interactionInstructions->softwareInterrupt(vm, value);
}


/**
 * Unconditional branch
 * Documented as instruction 18 in manual
 * Covers instructions with first byte 11100XXX
 * @param vm
 * @param instruction
 */
void tliUnconditionalBranch(VM_instance* vm, uint16_t instruction)
{
    uint16_t offset11 = instruction & 0b0000011111111111;

    // Calculate how to jump; we shift offset11 by 1, leading to a 12-bit number, then sign-extend it to 32 bits
    uint32_t relJump = offset11 << 1;
    relJump = (relJump & 0x00000FFF) | ((relJump & 0x0800) ? 0xFFFFF000 : 0);
    printf("B %d\n", relJump);
    uint32_t addr = vm_program_counter(vm) + relJump;

    // Jump to the new address
    vm_program_counter(vm) = addr;
}


/**
 * Long branch with link
 * Documented as instruction 19 in manual
 * Covers instructions with first byte 1111XXXX
 * @param vm
 * @param instruction
 */
void tliLongBranchWithLink(VM_instance* vm, uint16_t instruction)
{
    // This instruction actually always comes in pairs
    // The offset of the first half is stored in the LR for use by the second
    uint8_t high_or_low = (instruction & 0b0000100000000000) >> 11;
    uint16_t offset =     (instruction & 0b0000011111111111);

    if (high_or_low == 0) {
        // The first instruction; shift left by 12 bits, add it to the current PC, and store the result in LR
        vm_link_register(vm) = vm_program_counter(vm) + (offset << 12);
        printf("BL(0) %u\n", offset);
    } else {
        // The second instruction; put together the first half of the offset and the offset given in this instruction
        uint32_t targetAddress = vm_link_register(vm) + (offset << 1);

        // We put this new calculated address into PC and the address of the next instruction along in LR
        // Set the lowest bit of the link register because while irrelevant for us it will cause a switch to Thumb
        // mode if coming back from and ARM function.
        vm_link_register(vm) = vm_program_counter(vm) | 1;
        vm_program_counter(vm) = targetAddress;
        printf("BL(1) %u\n", offset);
    }
}



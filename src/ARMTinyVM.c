#include "ARMTinyVM.h"
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


#define i32_sign(n) (((n) & 0x80000000) >> 31)


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
        printf("UNKNOWN INSTRUCTION %x", instruction);
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
        printf("LSL R%u, R%u, #%u", rd, rs, offset5);
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
        printf("LSR R%u, R%u, #%u", rd, rs, offset5);
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
        printf("ASR R%u, R%u, #%u", rd, rs, offset5);
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
            printf("ADD r%u, r%u, #%u", rd, rs, rn);
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
            printf("SUB r%u, r%u, #%u", rd, rs, rn);
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
        printf("CMP r%u, #%u", rd, offset);

        // Set comparison registers based on Rd - Offset
        compareSetNZ(vm, vm->registers[rd] - offset);
        compareSetCV(vm, vm->registers[rd], 0 - offset);
    } else if (op == 0b10) {
        // ADD Rd, #Offset
        // Adds the offset to the register
        printf("ADD r%u, #%u", rd, offset);
        compareSetCV(vm, vm->registers[rd], offset);
        vm->registers[rd] += offset;
        compareSetNZ(vm, vm->registers[rd]);
    } else {
        // SUB Rd, #Offset
        // Subtracts the offset from the register
        printf("SUB r%u, #%u", rd, offset);
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
        printf("AND r%u, r%u", rd, rs);
        vm->registers[rd] = vm->registers[rd] & vm->registers[rs];
        compareSetNZ(vm, vm->registers[rd]);
    } else if (op == 0b0001) {
        // EOR Rd, Rs
        printf("EOR r%u, r%u", rd, rs);
        vm->registers[rd] = vm->registers[rd] ^ vm->registers[rs];
        compareSetNZ(vm, vm->registers[rd]);
    } else if (op == 0b0010) {
        // LSL Rd, Rs
        printf("LSL r%u, r%u", rd, rs);
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
        printf("LSR r%u, r%u", rd, rs);
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
        printf("ASR r%u, r%u", rd, rs);
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
            printf("ADD r%u, H%u", rd, 8+rs);
            vm->registers[rd] = vm->registers[rd] + vm->registers[8+rs];
        } else if (h1_and_2 == 0x10) {
            // ADD Hd, Rs
            // Sum together the values in high register Hd (8+Rd) and low register Rs
            // then store the result in Hd
            printf("ADD H%u, R%u", 8+rd, rs);
            vm->registers[8+rd] = vm->registers[rs];
        } else if (h1_and_2 == 0b11) {
            // ADD Hd, Hs
            // Sum together the values in the high registers Hd (8+Rd) and Hs (8+Rs)
            // then store the result in Hd
            printf("ADD H%u, H%u", 8+rd, 8+rs);
            vm->registers[8+rd] = vm->registers[8+rd] + vm->registers[8+rs];
        } else {
            printf("Invalid command %u", instruction);
            vm->finished = 0;
        }
    } else if (op == 0b01) {
        // TODO
    } else if (op == 0b10) {
        if (h1_and_2 == 0x01) {
            // MOV Rd, Hs
            // Moves a value from high register Hs (8+Rs) into low register Rd
            // rd <= 7 and (8+rs) <= 15, so this is safe
            printf("MOV r%u, r%u\n", rd, 8+rs);
            vm->registers[rd] = vm->registers[8+rs];
        } else if (h1_and_2 == 0x10) {
            // TODO
        } else if (h1_and_2 == 0x11) {
            // TODO
        } else {
            printf("Invalid command %u", instruction);
            vm->finished = 0;
        }
    } else {
        // TODO
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
    // TODO
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

    // Move the address of the next instruction into the link register
    vm_link_register(vm) = vm_program_counter(vm);

    // Trigger the interrupt
    vm->interactionInstructions->softwareInterrupt(vm, value);
}


/**
 * Unconditional brach
 * Documented as instruction 18 in manual
 * Covers instructions with first byte 11100XXX
 * @param vm
 * @param instruction
 */
void tliUnconditionalBranch(VM_instance* vm, uint16_t instruction)
{

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

}



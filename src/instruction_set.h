#ifndef INSTRUCTION_SET_H
#define INSTRUCTION_SET_H

// Instruction set top level calculations
#define istl_add_subtract(instr)        (((instr) & 0b1111100000000000) == 0b0001100000000000)
#define istl_mov_cmp_add_sub_imm(instr) (((instr) & 0b1110000000000000) == 0b0010000000000000)
#define istl_hi_reg_operations(instr)   (((instr) & 0b1111110000000000) == 0b0100010000000000)
#define istl_software_interrupt(instr)  (((instr) & 0b1111111100000000) == 0b1101111100000000)

#endif // INSTRUCTION_SET_H

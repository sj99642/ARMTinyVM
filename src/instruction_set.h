#ifndef INSTRUCTION_SET_H
#define INSTRUCTION_SET_H

// Instruction set top level calculations
#define istl_move_shifted_reg(instr)     ((((instr) & 0b11100000) == 0b00000000) && \
                                          (((instr) & 0b00011000) != 0b00011000))
#define istl_add_subtract(instr)          (((instr) & 0b11111000) == 0b00011000)
#define istl_mov_cmp_add_sub_imm(instr)   (((instr) & 0b11100000) == 0b00100000)
#define istl_alu_operations(instr)        (((instr) & 0b11111100) == 0b01000000)
#define istl_hi_reg_operations(instr)     (((instr) & 0b11111100) == 0b01000100)
#define istl_pc_relative_load(instr)      (((instr) & 0b11111000) == 0b01001000)
#define istl_load_with_reg_offset(instr)  (((instr) & 0b11110010) == 0b01010000)
#define istl_load_sgn_ext_byte(instr)     (((instr) & 0b11110010) == 0b01010010)
#define istl_load_imm_offset(instr)       (((instr) & 0b11100000) == 0b01100000)
#define istl_load_halfword(instr)         (((instr) & 0b11110000) == 0b10000000)
#define istl_sp_relative_load(instr)      (((instr) & 0b11110000) == 0b10010000)
#define istl_load_address(instr)          (((instr) & 0b11110000) == 0b10100000)
#define istl_add_offset_to_sp(instr)      (((instr) & 0b11111111) == 0b10110000)
#define istl_push_pop_registers(instr)    (((instr) & 0b11110110) == 0b10110100)
#define istl_multiple_load_store(instr)   (((instr) & 0b11110000) == 0b11000000)
#define istl_conditional_branch(instr)   ((((instr) & 0b11110000) == 0b11010000) && \
                                          (((instr) & 0b00001111) == 0b00001111))
#define istl_software_interrupt(instr)    (((instr) & 0b11111111) == 0b11011111)
#define istl_unconditional_branch(instr)  (((instr) & 0b11111000) == 0b11100000)
#define istl_long_branch_w_link(instr)    (((instr) & 0b11110000) == 0b11110000)

// The definitions of these different instructions are, more plainly:
//  1: istl_move_shifted_reg         000XXXXX XXXXXXXX
//  2: istl_add_subtract             00011XXX XXXXXXXX
//  3: istl_mov_cmp_add_sub_imm      001XXXXX XXXXXXXX
//  4: istl_alu_operations           010000XX XXXXXXXX
//  5: istl_hi_reg_operations        010001XX XXXXXXXX
//  6: istl_pc_relative_load         01001XXX XXXXXXXX
//  7: istl_load_with_reg_offset     0101XX0X XXXXXXXX
//  8: istl_load_sgn_ext_byte        0101XX1X XXXXXXXX
//  9: istl_load_imm_offset          011XXXXX XXXXXXXX
// 10: istl_load_halfword            1000XXXX XXXXXXXX
// 11: istl_sp_relative_load         1001XXXX XXXXXXXX
// 12: istl_load_address             1010XXXX XXXXXXXX
// 13: istl_add_offset_to_sp         10110000 XXXXXXXX
// 14: istl_push_pop_registers       1011X10X XXXXXXXX
// 15: istl_multiple_load_store      1100XXXX XXXXXXXX
// 16: istl_conditional_branch       1101XXXX XXXXXXXX
// 17: istl_software_interrupt       11011111 XXXXXXXX
// 18: istl_unconditional_branch     11100XXX XXXXXXXX
// 19: istl_long_branch_w_link       1111XXXX XXXXXXXX

// There are two potential conflicts:
// 1. Any number beginning 00011 is meant to be istl_add_subtract but could be
//    misinterpreted as istl_move_shifted_reg
// 2. The number 11011111 is meant to be istl_software_interrupt, but could be
//    misinterpreted as istl_conditional_branch.
// To prevent this, istl_move_shifted_reg and istl_conditional_branch have an extra term,
// excluding these special cases.

#endif // INSTRUCTION_SET_H

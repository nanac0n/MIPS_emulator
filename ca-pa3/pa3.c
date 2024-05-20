/**********************************************************************
 * Copyright (c) 2023
 *  Sang-Hoon Kim <sanghoonkim@ajou.ac.kr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTIABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 **********************************************************************/

#include <stdio.h>
#include <stdint.h>
#include "types.h"

/***
 * External entities in other files.
 */
extern struct stage stages[];		/* Pipelining stages */

extern unsigned char memory[];		/* Memory */

extern unsigned int registers[];	/* Registers */

extern unsigned int pc;				/* Program counter */

/**
 * Helper functions that might be useful. See main.c for the details of them
 */
extern bool is_noop(int stage);
extern void make_stall(int stage, int cycles);

unsigned int opcode__;
unsigned int types__;

/**********************************************************************
 * List of instructions that should be supported
 *
 * | Name   | Format   | Opcode / opcode + funct |
 * | ------ | -------- | ----------------------- |
 * | `add`  | r-format | 0 + 0x20                |
 * | `addi` | i-format | 0x08                    |
 * | `sub`  | r-format | 0 + 0x22                |
 * | `and`  | r-format | 0 + 0x24                |
 * | `andi` | i-format | 0x0c                    |
 * | `or`   | r-format | 0 + 0x25                |
 * | `ori`  | i-format | 0x0d                    |
 * | `nor`  | r-format | 0 + 0x27                |
 * | `sll`  | r-format | 0 + 0x00                |
 * | `srl`  | r-format | 0 + 0x02                |
 * | `sra`  | r-format | 0 + 0x03                |
 * | `lw`   | i-format | 0x23                    |
 * | `sw`   | i-format | 0x2b                    |
 * | `slt`  | r-format | 0 + 0x2a                |
 * | `slti` | i-format | 0x0a                    |
 * | `beq`  | i-format | 0x04                    |
 * | `bne`  | i-format | 0x05                    |
 * | `jr`   | r-format | 0 + 0x08                |
 * | `j`    | j-format | 0x02                    |
 * | `jal`  | j-format | 0x03                    |
 */

void IF_stage(struct IF_ID *if_id)
{
    /***
     * No need to check whether this stage is idle or not for some reasons...
     */

    /* TODO: Read one instruction in machine code from the memory */
    unsigned int machine_code = 0x0;

    machine_code |= memory[pc] << 24;    // 첫 번째 바이트 (가장 높은 주소)
    machine_code |= memory[pc + 1] << 16; // 두 번째 바이트
    machine_code |= memory[pc + 2] << 8;  // 세 번째 바이트
    machine_code |= memory[pc + 3];       // 네 번째 바이트 (가장 낮은 주소)

    printf("machine_code: %x\n", machine_code);

    /***
     * Set @stages[IF].instruction.machine_code with the read machine code
     * and @stages[IF].__pc with the current value of the program counter.
     * DO NOT REMOVE THOSE TWO STATEMENTS, as they are required by the
     * framework to work correctly.
     */
    stages[IF].instruction.machine_code = machine_code;
    stages[IF].__pc = pc;

    /* TODO: Fill in IF-ID interstage register */

    if_id -> instruction = machine_code;
    if_id -> next_pc = pc;
    pc += 4;

    /***
     * The framework processes @stage[IF].instruction.machine_code under
     * the hood to allow you to access human-readable stages[*].instruction
     * in the following stages. Check __run_cycle() in main.c.
     */
}


void ID_stage(struct IF_ID *if_id, struct ID_EX *id_ex)
{
    struct instruction *instr = &stages[ID].instruction;

    if (is_noop(ID)) return;

    /***
     * Register write should be taken place in WB_stage,
     * so actually there is nothing to do here for register write.
     */

    /* TODO: Process register read. May use if_id */

    instr->machine_code = if_id -> instruction;
    instr->opcode = (instr->machine_code >> 26) & 0x3f;
    opcode__ = instr->opcode;

    if(instr->opcode == 0){
        instr->type = r_type;
        types__ = r_type;
        instr->r_type.rs = (instr->machine_code >> 21) & 0x1f;
        instr->r_type.rt = (instr->machine_code >> 16) & 0x1f;
        instr->r_type.rd = (instr->machine_code >> 11) & 0x1f;
        instr->r_type.shamt = (instr->machine_code >> 6) & 0x1f;
        instr->r_type.funct = instr->machine_code & 0x3f;

        /* TODO: Fill in ID-EX interstage register */
        id_ex -> next_pc = if_id -> next_pc;
        id_ex -> reg1_value = instr->r_type.rs;
        id_ex -> reg2_value = instr->r_type.rt;
        id_ex -> instr_20_16 = instr->r_type.rd;
        id_ex -> immediate = instr -> r_type.shamt;
        id_ex -> instr_15_11 = instr -> r_type.funct;
    }
    else if(instr->opcode == 0x02 || instr->opcode == 0x03){
        instr->type = j_type;
        types__ = j_type;
        instr->j_type.target = instr->machine_code & 0x3ffffff;
    }
    else{
        instr->type = i_type;
        types__ = i_type;
        instr->i_type.rs = (instr->machine_code >> 21) & 0x1f;
        instr->i_type.rt = (instr->machine_code >> 16) & 0x1f;
        instr->i_type.imm = instr->machine_code & 0xffff;

        /* TODO: Fill in ID-EX interstage register */
        id_ex -> next_pc = if_id -> next_pc;
        id_ex -> reg1_value = instr->i_type.rs;
        id_ex -> reg2_value = instr->i_type.rt;
        id_ex -> immediate = instr -> i_type.imm;
    }

}

void EX_stage(struct ID_EX *id_ex, struct EX_MEM *ex_mem)
{
    struct instruction *instr = &stages[EX].instruction;
//    struct instruction *ID_instr = &stages[ID].instruction;
//
//    printf("type : %d\n", ID_instr->type);
//
//    printf("types: %d\n", types);

    if (is_noop(EX)) return;

    /* TODO: Good luck! */
    if (types__ == r_type){
        instr->r_type.rs = id_ex -> reg1_value;
        instr->r_type.rt = id_ex -> reg2_value;
        instr->r_type.rd = id_ex -> instr_20_16;
        instr->r_type.shamt = id_ex -> immediate;
        instr->r_type.funct = id_ex -> instr_15_11;

        switch (instr->r_type.funct) {
            case 0b100000:
                ex_mem -> alu_out = registers[instr->r_type.rs] + registers[instr->r_type.rt];
                break;
            case 0b100010:
                ex_mem -> alu_out = registers[instr->r_type.rs] - registers[instr->r_type.rt];
                break;
            case 0b100100:
                ex_mem -> alu_out = registers[instr->r_type.rs] & registers[instr->r_type.rt];
                break;
            case 0b100101:
                ex_mem -> alu_out = registers[instr->r_type.rs] | registers[instr->r_type.rt];
                break;
            case 0b100111:
                ex_mem -> alu_out = ~(registers[instr->r_type.rs] | registers[instr->r_type.rt]);
                break;
            case 0b000000:
                ex_mem -> alu_out = registers[instr->r_type.rt] << instr->r_type.shamt;
                break;
            case 0b000010:
                ex_mem -> alu_out = registers[instr->r_type.rt] >> instr->r_type.shamt;
                break;
            case 0b000011: { // sra
                int sig = (registers[instr->r_type.rt] >> 31) & 0x1;
                int temp = registers[instr->r_type.rt] >> instr->r_type.shamt;

                int mask = 0;

                if (sig) {
                    mask = (1 << (32 - instr->r_type.shamt)) - 1;
                    mask <<= instr->r_type.shamt;
                    ex_mem -> alu_out = temp | mask;
                }
                else{
                    ex_mem -> alu_out = registers[instr->r_type.rt] >> instr->r_type.shamt;
                }
                break;
            }
            case 0b101010:
                ex_mem -> alu_out = ((int) registers[instr->r_type.rs] < (int) registers[instr->r_type.rt] ? 1 : 0);
                break;
        }
        ex_mem -> write_reg = instr->r_type.rd;
    }
    else if(types__ == i_type){
        instr->i_type.rs = id_ex -> reg1_value;
        instr->i_type.rt = id_ex -> reg2_value;
        instr->i_type.imm = id_ex -> immediate;

        switch (opcode__) {
            case 0b001000:
                ex_mem -> alu_out = registers[instr->i_type.rs] + (int)instr->i_type.imm;
                ex_mem -> write_reg = instr->i_type.rt;
                break;
            case 0b001100:
                ex_mem -> alu_out = registers[instr->i_type.rs] & (int)instr->i_type.imm;
                ex_mem -> write_reg = instr->i_type.rt;
                break;
            case 0b001101:
                ex_mem -> alu_out = registers[instr->i_type.rs] | (int)instr->i_type.imm;
                ex_mem -> write_reg = instr->i_type.rt;
                break;
            case 0b001010:
                ex_mem -> alu_out = (registers[instr->i_type.rs] < (int)instr->i_type.imm ? 1 : 0);
                ex_mem -> write_reg = instr->i_type.rt;
                break;
            case 0b101011: // sw
                ex_mem->alu_out = registers[id_ex->reg1_value] + (int)id_ex->immediate;
                break;
            case 0b100011: // lw
                ex_mem->alu_out = registers[id_ex->reg1_value] + (int)id_ex->immediate;
                ex_mem->write_reg = id_ex->reg2_value;
                break;
        }
    }
    ex_mem -> next_pc = id_ex -> next_pc;
}


void MEM_stage(struct EX_MEM *ex_mem, struct MEM_WB *mem_wb)
{
    struct instruction *instr = &stages[MEM].instruction;
    if (is_noop(MEM)) return;


    if(opcode__ == 0b100011){ //lw
        int address = ex_mem -> alu_out;
        unsigned int word = 0;
        for (int i=0; i<4; i++){
            word = (word << 8) | memory[address + i];
        }
        mem_wb -> mem_out = word;
        mem_wb -> write_reg = ex_mem -> write_reg;
    }
    else if(opcode__ == 0b101011){ //sw
        unsigned int address = ex_mem->alu_out;
        unsigned int word = ex_mem->write_value;
        for (int i = 0; i < 4; i++) {
            memory[address + i] = (word >> (24 - 8 * i)) & 0xFF;
        }
    }
    else{
        mem_wb -> alu_out = ex_mem -> alu_out;
        mem_wb-> write_reg = ex_mem-> write_reg;
    }
}


void WB_stage(struct MEM_WB *mem_wb)
{
    struct instruction *instr = &stages[WB].instruction;


    if (is_noop(WB)) return;

    /* TODO: Fingers crossed */

    if (opcode__ == 0b100011){ //lw
        registers[mem_wb->write_reg] = mem_wb->mem_out;
    }
    else{
        registers[mem_wb->write_reg] = mem_wb->alu_out;
    }
}

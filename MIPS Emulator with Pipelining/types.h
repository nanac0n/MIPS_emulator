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

#ifndef __PIPESIM_TYPES_H__
#define __PIPESIM_TYPES_H__

typedef unsigned char bool;
#define true	1
#define false	0

enum pipesim_constants {
	IF = 0,
	ID,
	EX,
	MEM,
	WB,

	NR_STAGES = 5,
};

enum instruction_type {
	unknown_type = 0,
	r_type,
	i_type,
	j_type,
};

struct instruction {
	unsigned int machine_code;
	unsigned int opcode;
	char *name;
	enum instruction_type type;

	union {
		struct {
			unsigned int rs;
			unsigned int rt;
			unsigned int rd;
			unsigned int shamt;
			unsigned int funct;
		} r_type;

		struct {
			unsigned int rs;
			unsigned int rt;
			unsigned int imm;
		} i_type;

		struct {
			unsigned int target;
		} j_type;
	};
};


/* Pipelining stages. Use @instruction as the control path */
struct stage {
	struct instruction instruction;
	unsigned int __pc;	/* DO NOT ACCESS THIS VARIABLE */
	unsigned int nr_stalls;
};


/* Inter-stage registers */
struct IF_ID {
	unsigned int instruction;
	unsigned int next_pc;
};

struct ID_EX {
	unsigned int next_pc;

	unsigned int reg1_value;
	unsigned int reg2_value;

	unsigned int immediate;
	unsigned int instr_20_16;
	unsigned int instr_15_11;
};

struct EX_MEM {
	unsigned int next_pc;

	unsigned int alu_out;
	unsigned int write_value;

	unsigned int write_reg;
};

struct MEM_WB {
	unsigned int mem_out;
	unsigned int alu_out;

	unsigned int write_reg;
};

#endif

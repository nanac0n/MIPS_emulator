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
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <inttypes.h>
#include <ctype.h>

#include "types.h"

/* To avoid security error on Visual Studio */
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4996)

#define MAX_NR_TOKENS	32	/* Maximum length of tokens in a command */
#define MAX_TOKEN_LEN	64	/* Maximum length of single token */
#define MAX_COMMAND		256 /* Maximum length of command string */

/**
 * memory[] emulates the memory of the machine
 */
unsigned char memory[1 << 20] = {	/* 1MB memory at 0x0000 0000 -- 0x0100 0000 */
	0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
	0xde, 0xad, 0xbe, 0xef, 0x00, 0x00, 0x00, 0x00,
	'h',  'e',  'l',  'l',  'o',  ' ',  'w',  'o',
	'r',  'l',  'd',  '!',  '!',  0x00, 0x00, 0x00,
	'a',  'w',  'e',  's',  'o',  'm',  'e',  ' ',
	'c',  'o',  'm',  'p',  'u',  't',  'e',  'r',
	' ',  'a',  'r',  'c',  'h',  'i',  't',  'e',
	'c',  't',  'u',  'r',  'e',  '!',  0x00, 0x00,
};

#define INITIAL_PC	0x1000	/* Initial value for PC register */
#define INITIAL_SP	0x8000	/* Initial location for stack pointer */

/**
 * Registers of the machine
 */
unsigned int registers[32] = {
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0x10, INITIAL_PC, 0x20, 2, 0xbadacafe, 0xcdcdcdcd, 0xffffffff, 7,
	0, 0, 0, 0, 0, INITIAL_SP, 0, 0,
};

/**
 * Names of the registers. Note that $zero is shorten to zr
 */
const char *register_names[] = {
	"zr", "at", "v0", "v1", "a0", "a1", "a2", "a3",
	"t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
	"s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
	"t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra"
};

/**
 * Program counter register
 */
unsigned int pc = INITIAL_PC;

/**
 * Pipelining
 */
struct stage stages[NR_STAGES];

static const char *stage_name[] = {
	"IF", "ID", "EX", "MEM", "WB",
};


/**
 * Cycles executed so far
 */
static int __cycles = 0;

/**
 * Execution behavior parameters
 */
static bool __verbose = false;
static bool __verbose_memory = false;
static bool __auto_run = false;
static const int __dump_interval = 10;


/**
 * MIPS instruction set
 */
struct mips_instruction {
	char *name;
	enum instruction_type type;
	union {
		unsigned int opcode;
		unsigned int funct;
	};
};

static struct mips_instruction mips_instruction_set[] = {
	[0x00] = { "r-type", r_type, },
	[0x08] = { "addi", i_type, },
	[0x0c] = { "andi", i_type, },
	[0x0d] = { "ori", i_type, },
	[0x23] = { "lw", i_type, },
	[0x2b] = { "sw", i_type, },
	[0x04] = { "beq", i_type, },
	[0x05] = { "bne", i_type, },
	[0x0a] = { "slti", i_type, },
	[0x02] = { "j", j_type, },
	[0x03] = { "jal", j_type, },
};

static struct mips_instruction r_type_instructions[] = {
	[0x20] = { "add", r_type, },
	[0x22] = { "sub", r_type, },
	[0x24] = { "and", r_type, },
	[0x25] = { "or", r_type, },
	[0x27] = { "nor", r_type, },
	[0x00] = { "sll", r_type, },
	[0x02] = { "srl", r_type, },
	[0x03] = { "sra", r_type, },
	[0x08] = { "jr", r_type, },
	[0x2a] = { "slt", r_type, },
};

static bool __parse_instruction(unsigned int instr, struct instruction *pi)
{
	unsigned int opcode = instr >> 26;

	switch (mips_instruction_set[opcode].type) {
	case r_type:
		*pi = (struct instruction) {
			.type = r_type,
			.r_type = {
				.rs = (instr >> 21) & 0x1f,
				.rt = (instr >> 16) & 0x1f,
				.rd = (instr >> 11) & 0x1f,
				.shamt = (instr >> 6) & 0x1f,
				.funct = instr & 0x3f,
			},
		};
		pi->name = r_type_instructions[pi->r_type.funct].name;
		break;
	case i_type:
		*pi = (struct instruction) {
			.name = mips_instruction_set[opcode].name,
			.type = i_type,
			.i_type = {
				.rs = (instr >> 21) & 0x1f,
				.rt = (instr >> 16) & 0x1f,
				.imm = (instr & 0x0000ffff),
			},
		};
		break;
	case j_type:
		*pi = (struct instruction) {
			.name = mips_instruction_set[opcode].name,
			.type = j_type,
			.j_type = {
				.target = instr & 0x03ffffff,
			},
		};
		break;
	default:
		printf("UNKNOWN INSTRUCTION %08x with opcode %x\n", instr, opcode);
		exit(-1);
		break;
	}

	pi->machine_code = instr;
	pi->opcode = opcode;

	return pi->type != unknown_type;
}


static inline bool strmatch(char * const str, const char *expect)
{
	return (strlen(str) == strlen(expect)) && (strncmp(str, expect, strlen(expect)) == 0);
}


static void __pipeline_stat(void)
{
	fprintf(stderr, "\n### %d ###\n", __cycles);
	for (int i = 0; i < NR_STAGES; i++) {
		fprintf(stderr, "%3s: 0x%08x  0x%08x  %d\n", stage_name[i],
			stages[i].instruction.machine_code, stages[i].__pc, stages[i].nr_stalls);
	}
	fprintf(stderr, "\n");
}

static void __show_registers(char * const register_name)
{
	int from = 0, to = 0;
	bool include_pc = false;

	if (strmatch(register_name, "all")) {
		from = 0;
		to = 32;
		include_pc = true;
	} else if (strmatch(register_name, "pc")) {
		include_pc = true;
	} else {
		for (int i = 0; i < sizeof(register_names) / sizeof(*register_names); i++) {
			if (strmatch(register_name, register_names[i])) {
				from = i;
				to = i + 1;
			}
		}
	}

	for (int i = from; i < to; i++) {
		fprintf(stderr, "[%02d:%2s] 0x%08x    %u\n", i, register_names[i], registers[i], registers[i]);
	}
	if (include_pc) {
		fprintf(stderr, "[  pc ] 0x%08x\n", pc);
	}
}

static void __dump_memory(unsigned int addr, size_t length)
{
    for (size_t i = 0; i < length; i += 4) {
        fprintf(stderr, "0x%08lx:  %02x %02x %02x %02x    %c %c %c %c\n",
				addr + i,
                memory[addr + i    ], memory[addr + i + 1],
                memory[addr + i + 2], memory[addr + i + 3],
                isprint(memory[addr + i    ]) ? memory[addr + i    ] : '.',
				isprint(memory[addr + i + 1]) ? memory[addr + i + 1] : '.',
				isprint(memory[addr + i + 2]) ? memory[addr + i + 2] : '.',
				isprint(memory[addr + i + 3]) ? memory[addr + i + 3] : '.');
    }
}


/**
 * Pipeline registers
 */
static struct IF_ID if_id = { 0 };
static struct ID_EX id_ex = { 0 };
static struct EX_MEM ex_mem = { 0 };
static struct MEM_WB mem_wb = { 0 };


/**
 * Pipelining stages to be completed
 */
extern void IF_stage(struct IF_ID *if_id);
extern void ID_stage(struct IF_ID *if_id, struct ID_EX *id_ex);
extern void EX_stage(struct ID_EX *id_ex, struct EX_MEM *ex_mem);
extern void MEM_stage(struct EX_MEM *ex_mem, struct MEM_WB *mem_wb);
extern void WB_stage(struct MEM_WB *mem_wb);


/**********************************************************************
 * is_noop(stage)
 *
 * DESCRIPTION
 *   Determine whether the stage @stage is currently idle/noop
 *
 * RETURN
 *   true if the stage @stage has nothing to do in this cycle
 *   false otherwise
 */
bool is_noop(int stage)
{
	struct stage *s = &stages[stage];
	return s->instruction.machine_code == 0 && s->__pc == 0;
}

static bool __is_program_finished(void)
{
	int nr_idle_stages = 0;

	/* Count idle stages */
	for (int i = 0; i < NR_STAGES; i++) {
		struct stage *s = stages + i;
		if (s->instruction.machine_code == 0x00 && s->__pc != 0) {
			nr_idle_stages++;
		}
	}

	/* If all stages are idle, the program is finished */
	return nr_idle_stages < NR_STAGES;
}


/**********************************************************************
 * make_stall(stage, cycles)
 *
 * DESCRIPTION
 *   Make stage @stage to be stalled for @cycles cycles. Note that the
 *   earlier stages are also influenced.
 */
void make_stall(int stage, int cycles)
{
	struct stage *s = stages + stage;
	s->nr_stalls += cycles;
}

static bool __should_stall(int stage)
{
	struct stage *s = &stages[stage];

	if (!(s->nr_stalls)) return false;

	s->nr_stalls--;
	return !!s->nr_stalls;
}

/**********************************************************************
 * __run_cycle()
 *
 * DESCRIPTION
 *   Simulate one CPU cycle.
 *
 * RETURN
 *   true if the pipeline is not empty.
 *   false if the pipeline is empty (i.e., nothing to process anymore).
 */
static bool __run_cycle(void)
{
	/**
	 * Prepare stages for this cycle. Inject noop into stalled stages without
	 * moving related pipeline registers.
	 */
	for (int i = NR_STAGES - 1; i > 0; i--) {
		if (!__should_stall(i)) {
			stages[i] = (struct stage) {
				.instruction = stages[i - 1].instruction,
				.__pc = stages[i - 1].__pc,
				.nr_stalls = stages[i].nr_stalls,
			};
		} else {
			stages[i] = (struct stage) { /* Inject noop */
				.instruction = { 0 },
				.__pc = 0,
				.nr_stalls = stages[i].nr_stalls,
			};
			break;
		}
	}

	/**
	 * Invoke each stage one at a time. Note that the calling takes place **in
	 * the reverse order** so that the output of an stage is processed by the
	 * next stage properly.
	 */
	WB_stage(&mem_wb);

	if (stages[MEM].nr_stalls) goto done;
	MEM_stage(&ex_mem, &mem_wb);

	if (stages[EX].nr_stalls) goto done;
	EX_stage(&id_ex, &ex_mem);

	if (stages[ID].nr_stalls) goto done;
	/**
	 * Parse the machine code read from IF stage
	 */
	__parse_instruction(stages[ID].instruction.machine_code, &stages[ID].instruction);
	ID_stage(&if_id, &id_ex);

	/**
	 * Handle IF stage stalls. It required extra attentions X-D
	 */
	if (__should_stall(IF)) {
		stages[IF] = (struct stage) { /* Inject noop */
			.instruction = { 0 },
			.__pc = 0,
			.nr_stalls = stages[IF].nr_stalls,
		};
	} else {
		IF_stage(&if_id);
	}

done:

	/**
	 * This cycle is done. Print out the current status to check
	 */
	__cycles++;

	__pipeline_stat();
	if (__verbose || __cycles % __dump_interval == 0) __show_registers("all");
	if (__verbose_memory) __dump_memory(0x0, 16);

	return __is_program_finished();
}


/**********************************************************************
 * __run_program(nr_cycles)
 *
 * DESCRIPTION
 *   Start running the program from the current @pc. If @nr_cycles is 0,
 *   the framework runs to the end of the program, until @__run_cycle()
 *   returns false. When @nr_cycles is non-zero, it runs up to @nr_cycles.
 *
 * RETURN
 *   0
 */
static int __run_program(unsigned int nr_cycles)
{
	unsigned int cycles = 0;

	while (nr_cycles == 0 || (cycles < nr_cycles)) {
		if (!__run_cycle()) break;
		cycles++;
	}

	if (nr_cycles && cycles == nr_cycles) {
		fprintf(stderr, "MAXIMUM CYCLES REACHED\n");
	}
	return 0;
}

/**********************************************************************
 * __load_program(filename)
 *
 * DESCRIPTION
 *   Load the instructions in the file @filename onto the memory starting at
 *   @INITIAL_PC. Each line in the program file looks like;
 *
 *	 [MIPS instruction started with 0x prefix]  // optional comments
 *
 *   For example,
 *
 *   0x8c090008
 *   0xac090020	// sw t1, zero + 32
 *   0x8c080000
 *
 *   implies three MIPS instructions to load. Each machine instruction may
 *   be followed by comments like the second instruction, which are ignored.
 *
 * RETURN
 *	 0 on successfully load the program
 *	 any other value otherwise
 */
static int __load_program(char * const filename)
{
	char buffer[80];
	unsigned int addr = INITIAL_PC;
	unsigned int nr_instructions = 0;
	FILE *file = fopen(filename, "r");

	printf("- Loading %s...\n", filename);

	if (!file) {
		printf("File %s does not exist\n", filename);
		return -EINVAL;
	}

	while (fgets(buffer, sizeof(buffer), file)) {
		unsigned int instr = strtoimax(buffer, NULL, 0);

		if (__verbose) {
			struct instruction in;
			__parse_instruction(instr, &in);
			printf("  %3d: 0x%08x  %s\n", nr_instructions, instr, in.name);
		}

		memory[addr++] = (unsigned char)((instr & 0xff000000) >> 24);
        memory[addr++] = (unsigned char)((instr & 0x00ff0000) >> 16);
        memory[addr++] = (unsigned char)((instr & 0x0000ff00) >>  8);
        memory[addr++] = (unsigned char)((instr & 0x000000ff) >>  0);

		nr_instructions++;
	}
	fclose(file);
	if (__verbose) printf("\n");

	printf("- %d instruction%s loaded\n", nr_instructions,
			nr_instructions < 2 ? "" : "s");
	printf("\n");
	return 0;
}

static void __process_command(int argc, char *argv[])
{
	if (argc == 0) return;

	if (strmatch(argv[0], "run") || strmatch(argv[0], "r")) {
		if (argc == 1) {
			__run_program(0);
		} else if (argc == 2) {
			__run_program(atoi(argv[1]));
		} else {
			printf("Usage: run [cycles to run]\n");
		}
	} else if (strmatch(argv[0], "show")) {
		if (argc == 1) {
			__show_registers("all");
		} else if (argc == 2) {
			__show_registers(argv[1]);
		} else {
			printf("Usage: show { [register name] }\n");
		}
	} else if (strmatch(argv[0], "dump")) {
		if (argc == 3) {
			__dump_memory(strtoimax(argv[1], NULL, 0), strtoimax(argv[2], NULL, 0));
		} else {
			printf("Usage: dump [start address] [length]\n");
		}
	} else if (strmatch(argv[0], "pipe")) {
		__pipeline_stat();
	} else if (strmatch(argv[0], "reset")) {
		__cycles = 0;
		pc = INITIAL_PC;
	} else if (strmatch(argv[0], "next") || strmatch(argv[0], "n")) {
		__run_cycle();
	}
}

static int __parse_command(char *command, int *nr_tokens, char *tokens[])
{
	char *curr = command;
	int token_started = false;
	*nr_tokens = 0;

	while (*curr != '\0') {
		if (isspace(*curr)) {
			*curr = '\0';
			token_started = false;
		} else {
			if (!token_started) {
				tokens[*nr_tokens] = curr;
				*nr_tokens += 1;
				token_started = true;
			}
		}
		curr++;
	}

	/* Exclude comments from tokens */
	for (int i = 0; i < *nr_tokens; i++) {
		if (strmatch(tokens[i], "//") || strmatch(tokens[i], "#")) {
			*nr_tokens = i;
			tokens[i] = NULL;
		}
	}

	return 0;
}

int main(int argc, char * const argv[])
{
	char command[MAX_COMMAND] = {'\0'};
	int opt;
	char *input_file = "testcases/program-r";
	unsigned int max_cycles = 0;

	while ((opt = getopt(argc, argv, "c:vmr")) != -1) {
		switch (opt) {
		case 'c':
			max_cycles = atol(optarg);
			break;
		case 'v':
			__verbose = true;
			break;
		case 'm':
			__verbose_memory = true;
			break;
		case 'r':
			__auto_run = true;
			break;
		}
	}

	/*
	if (argc == optind) {
		fprintf(stderr, "No input file specified\n");
		return EXIT_FAILURE;
	}
	*/
	if (argc != optind) {
		input_file = argv[optind];
	}

	if (__load_program(input_file)) {
		return EXIT_FAILURE;
	}

	if (__auto_run) {
		__run_program(max_cycles);
		if (!__verbose && __cycles % __dump_interval != 0) {
			__show_registers("all");
		}
		return EXIT_SUCCESS;
	}

	printf("**************************************\n");
	printf("  MIPS Emulator with Pipelining v0.1\n");
	printf("\n");
	printf(">> ");

 	while (fgets(command, sizeof(command), stdin)) {
		char *tokens[MAX_NR_TOKENS] = { NULL };
		int nr_tokens = 0;

		for (size_t i = 0; i < strlen(command); i++) {
			command[i] = tolower(command[i]);
		}

		if (__parse_command(command, &nr_tokens, tokens) < 0)
			continue;

		__process_command(nr_tokens, tokens);

		printf(">> ");
	}

	return EXIT_SUCCESS;
}

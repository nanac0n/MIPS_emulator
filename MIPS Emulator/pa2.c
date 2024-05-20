/**********************************************************************
 * Copyright (c) 2019-2023
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
#include <errno.h>
#include <string.h>
#include <inttypes.h>
#include <ctype.h>

/*====================================================================*/
/*          ****** DO NOT MODIFY ANYTHING FROM THIS LINE ******       */

/* To avoid security error on Visual Studio */
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4996)

#define MAX_NR_TOKENS	32	/* Maximum length of tokens in a command */
#define MAX_TOKEN_LEN	64	/* Maximum length of single token */
#define MAX_COMMAND	256 /* Maximum length of command string */

typedef unsigned char bool;
#define true	1
#define false	0

const char *__color_start = "[1;32;40m";
const char *__color_end = "[0m";

/**
 * memory[] emulates the memory of the machine
 */
static unsigned char memory[1 << 20] = {	/* 1MB memory at 0x0000 0000 -- 0x0100 0000 */
	0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
	0xde, 0xad, 0xbe, 0xef, 0x00, 0x00, 0x00, 0x00,
	'h',  'e',  'l',  'l',  'o',  ' ',  'w',  'o',
	'r',  'l',  'd',  '!',  '!',  0x00, 0x00, 0x00,
	'a',  'w',  'e',  's',  'o',  'm',  'e',  ' ',
	'c',  'o',  'm',  'p',  'u',  't',  'e',  'r',
	' ',  'a',  'r',  'c',  'h',  'i',  't',  'e',
	'c',  't',  'u',  'r',  'e',  '.',  0x00, 0x00,
};

#define INITIAL_PC	0x1000	/* Initial value for PC register */
#define INITIAL_SP	0x8000	/* Initial location for stack pointer */

/**
 * Registers of the machine
 */
static unsigned int registers[32] = {
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0x10, INITIAL_PC, 0x20, 3, 0xbadacafe, 0xcdcdcdcd, 0xffffffff, 7,
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
static unsigned int pc = INITIAL_PC;

/**
 * strmatch()
 *
 * DESCRIPTION
 *   Compare strings @str and @expect and return 1 if they are the same.
 *   You may use this function to simplify string matching :)
 *
 * RETURN
 *   1 if @str and @expect are the same
 *   0 otherwise
 */
static inline bool strmatch(char * const str, const char *expect)
{
	return (strlen(str) == strlen(expect)) && (strncmp(str, expect, strlen(expect)) == 0);
}

/*          ****** DO NOT MODIFY ANYTHING UP TO THIS LINE ******      */
/*====================================================================*/


/**********************************************************************
 * process_instruction
 *
 * DESCRIPTION
 *   Execute the machine code given through @instr. The following table lists
 *   up the instructions to support. Note that a pseudo instruction 'halt'
 *   (0xffffffff) is added for the testing purpose. Also '*' instrunctions are
 *   the ones that are newly added to PA2.
 *
 * | Name   | Format    | Opcode / opcode + funct |
 * | ------ | --------- | ----------------------- |
 * | `add`  | r-format  | 0 + 0x20                |
 * | `addi` | i-format  | 0x08                    |
 * | `sub`  | r-format  | 0 + 0x22                |
 * | `and`  | r-format  | 0 + 0x24                |
 * | `andi` | i-format  | 0x0c                    |
 * | `or`   | r-format  | 0 + 0x25                |
 * | `ori`  | i-format  | 0x0d                    |
 * | `nor`  | r-format  | 0 + 0x27                |
 * | `sll`  | r-format  | 0 + 0x00                |
 * | `srl`  | r-format  | 0 + 0x02                |
 * | `sra`  | r-format  | 0 + 0x03                |
 * | `lw`   | i-format  | 0x23                    |
 * | `sw`   | i-format  | 0x2b                    |
 * | `slt`  | r-format* | 0 + 0x2a                |
 * | `slti` | i-format* | 0x0a                    |
 * | `beq`  | i-format* | 0x04                    |
 * | `bne`  | i-format* | 0x05                    |
 * | `jr`   | r-format* | 0 + 0x08                |
 * | `j`    | j-format* | 0x02                    |
 * | `jal`  | j-format* | 0x03                    |
 * | `halt` | special*  | @instr == 0xffffffff    |
 *
 * RETURN VALUE
 *   1 if successfully processed the instruction.
 *   0 if @instr is 'halt' or unknown instructions
 */
static int process_instruction(unsigned int instr)
{
	char binary[100]={'\0',};
	char opcode[10]={'\0',}; char rs[10]={'\0',}; char rt[10]={'\0',}; 
	char rd[10]={'\0',}; char shamt[10]={'\0',}; char funct[10]={'\0',};
	char imm[100]={'\0',};
	char addr[100]={'\0',};
	char hex_chars[100], hex_value[100];  
	int immed_hex = 0;
	int rs_num=0, rt_num=0, rd_num=0, shamt_num=0, immed_num=0,addr_num=0;
	unsigned int bitmask = 1 << 31;
	
	for (int i=0; i<32; i++){ // 16진수 -> 2진수
		if(instr & bitmask) strcat(binary, "1");
		else strcat(binary, "0");
		instr <<=1;
	}
	for(int i=0; i<6; i++){
		opcode[i] = binary[i];
	}

	if(!strcmp(opcode, "000000")){ // R-format
		for(int i=0; i<5; i++){
			rs[i] = binary[i+6];
			rt[i] = binary[i+11];
			rd[i] = binary[i+16];
		}
		rs_num = strtol(rs, NULL, 2);
		rt_num = strtol(rt, NULL, 2);
		rd_num = strtol(rd, NULL, 2);
		
		for(int i=0; i<5; i++){
			shamt[i] = binary[i+21];
		}
		shamt_num = strtol(shamt, NULL, 2);

		for(int i=0; i<6; i++){
			funct[i] = binary[i+26];
		}
	}
	else if(!strcmp(opcode, "000010") || !strcmp(opcode, "000011")){ // J-format
		for(int i=0; i<26; i++){
			addr[i] = binary[i+6];
		}
		addr_num = strtol(addr, NULL, 2);

	}
	else{ // I-format
		for(int i=0; i<5; i++){
			rs[i] = binary[i+6];
			rt[i] = binary[i+11];
		}
		rs_num = strtol(rs, NULL, 2);
		rt_num = strtol(rt, NULL, 2);

		for(int i=0; i<16; i++){
			imm[i] = binary[i+16];
		}

		immed_num = strtol(imm, NULL, 2); // 2진수


        for (int i = 0; i < 4; i++) {
            unsigned short int bin_to_hex = (immed_num >> (12 - 4 * i)) & 0xF; //
            if (bin_to_hex<= 9) {
                hex_value[i] = '0' + bin_to_hex;
            } else {
                hex_value[i] = 'A' + (bin_to_hex - 10);
            }
        }

		immed_hex = strtol(hex_value, NULL, 16);		

	}

	if(!strcmp(funct, "100000")){ //add
		registers[rd_num] = registers[rs_num] + registers[rt_num];
	}
	else if(!strcmp(funct, "100010")){ //sub
		registers[rd_num] = registers[rs_num] - registers[rt_num];
	}
	else if(!strcmp(funct, "100100")){ //and
		registers[rd_num] = registers[rs_num] & registers[rt_num];
	}
	else if(!strcmp(funct, "100101")){ //or
		registers[rd_num] = registers[rs_num] | registers[rt_num];
	}
	else if(!strcmp(funct, "100111")){ //nor
		registers[rd_num] = ~(registers[rs_num] | registers[rt_num]);
	}
	else if(!strcmp(funct, "000000")){ //sll
		registers[rd_num] = registers[rt_num] << shamt_num;
	}
	else if(!strcmp(funct, "000010")){ //srl
		registers[rd_num] = registers[rt_num] >> shamt_num;
	}
	else if(!strcmp(funct, "000011")){ //sra
        int sign_bit = (registers[rt_num] >> 31) & 0x1;
        int temp = registers[rt_num] >> shamt_num;

        int mask = 0;
        if (sign_bit) {
            mask = (1 << (32 - shamt_num)) - 1;
            mask <<= shamt_num;
        }
        registers[rd_num] = temp | mask;
    }
	else if(!strcmp(funct, "101010")){ //slt
        int rs_value = 0;
        rs_value = (int)registers[rs_num];
        int rt_value = 0;
        rt_value = (int)registers[rt_num];
		registers[rd_num] = (rs_value < rt_value) ? 1 : 0;
	}
	else if(!strcmp(funct, "001000")){ //jr
		pc = registers[rs_num];
	}
	else if(!strcmp(opcode, "000010")){ //j
		pc = (pc & 0xf0000000) | (addr_num << 2);
	}
	else if(!strcmp(opcode, "000011")){ //jal
		registers[31] = pc;
		pc = (pc & 0xf0000000) | (addr_num << 2);
	}
	else if(!strcmp(opcode, "000100")){ //beq
		if(registers[rs_num] == registers[rt_num]) {
            int16_t signed_immed = (int16_t)immed_num;
            pc = pc + (signed_immed << 2);
        }
	}
	else if(!strcmp(opcode, "000101")){ //bne
		if(registers[rs_num] != registers[rt_num]) {
            int16_t signed_immed = (int16_t)immed_num;
            pc = pc + (signed_immed << 2);
        }
	}
	else if(!strcmp(opcode, "001000")){ //addi
        int16_t signed_immed = (int16_t)immed_num;
		registers[rt_num] = registers[rs_num] +signed_immed;
	}
	else if(!strcmp(opcode, "001100")){ //andi
		registers[rt_num] = registers[rs_num] & immed_num;
	}
	else if(!strcmp(opcode, "001101")){ //ori
		registers[rt_num] = registers[rs_num] | immed_num;
	}

	else if(!strcmp(opcode, "100011")){ //lw
        int address = 0;
        address = registers[rs_num] + immed_hex;
        unsigned int word = 0;
        for (int i = 0; i < 4; i++) {
            word = (word << 8) | memory[address + i];
        }
        registers[rt_num] = word;
	}
	else if(!strcmp(opcode, "101011")){ //sw
        unsigned int address = 0;
        address = registers[rs_num] + immed_hex;
        unsigned int word = registers[rt_num];

        for (int i = 0; i < 4; i++) {
            memory[address + i] = (word >> (24 - 8 * i)) & 0xFF;
        }
	}

	else if(!strcmp(opcode, "001010")){ //slti
		registers[rt_num] = registers[rs_num] < immed_num ? 1 : 0;
	}
	else{ //halt
        return 0;
	}
	return 0;
}


/**********************************************************************
 * load_program(filename)
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
*   be followed by comments like the second instruction. However you can simply
*   call strtoimax(linebuffer, NULL, 0) to read the machine code while
*   ignoring the comment parts.
*
*	 The program DOES NOT include the 'halt' instruction. Thus, make sure the
*	 'halt' instruction is appended to the loaded instructions to terminate
        *	 your program properly.
*
*	 Refer to the @main() for reading data from files. (fopen, fgets, fclose).
*
* RETURN
*	 0 on successfully load the program
        *	 any other value otherwise
*/

static int load_program(char * const filename) {
    char linebuffer[100];
    FILE *fp = fopen(filename, "r");
    unsigned int hexvalue = 0;
    pc = INITIAL_PC;

    if (fp == NULL) {
        printf("Error opening file!\n");
        return -EINVAL;
    }

    while (fgets(linebuffer, sizeof(linebuffer), fp)) {
        hexvalue = strtoimax(linebuffer, NULL, 0);

        for (int i = 0; i < 4; i++) {
            memory[pc + (3 - i)] = (hexvalue >> (8 * i)) & 0xff;
        }
        pc += 4;
    }

    for (int i = 0; i < 4; i++) {
        memory[pc + (3 - i)] = 0xff;
    }
    fclose(fp);
    return 0;

}
/**********************************************************************
 * run_program
 *
 * DESCRIPTION
 *   Start running the program that is loaded by @load_program function above.
 *   If you implement @load_program() properly, the first instruction is placed
 *   at @INITIAL_PC. Using @pc, which is the program counter of this processor,
 *   you can emulate the MIPS processor by
 *
 *   1. Read instruction from @pc
 *   2. Increment @pc by 4
 *   3. Call @process_instruction(instruction)
 *   4. Repeat until @process_instruction() returns 0
 *
 * RETURN
 *   0
 */
static int run_program(void) {
    unsigned int instruct;
    pc = INITIAL_PC;

    while (1) {
        instruct = (memory[pc] << 24) | (memory[pc + 1] << 16) |
                      (memory[pc + 2] << 8) | memory[pc + 3];

        if (instruct == 0xffffffff) {
            pc += 4;
            break;
        }
        pc += 4;
        process_instruction(instruct);
    }
    return 0;
 }


/*====================================================================*/
/*          ****** DO NOT MODIFY ANYTHING FROM THIS LINE ******       */
    static void __show_registers(char *const register_name) {
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

    static void __dump_memory(unsigned int addr, size_t length) {
        for (size_t i = 0; i < length; i += 4) {
            fprintf(stderr, "0x%08lx:  %02x %02x %02x %02x    %c %c %c %c\n",
                    addr + i,
                    memory[addr + i], memory[addr + i + 1],
                    memory[addr + i + 2], memory[addr + i + 3],
                    isprint(memory[addr + i]) ? memory[addr + i] : '.',
                    isprint(memory[addr + i + 1]) ? memory[addr + i + 1] : '.',
                    isprint(memory[addr + i + 2]) ? memory[addr + i + 2] : '.',
                    isprint(memory[addr + i + 3]) ? memory[addr + i + 3] : '.');
        }
    }

    static void __process_command(int argc, char *argv[]) {
        if (argc == 0) return;

        if (strmatch(argv[0], "load")) {
            if (argc == 2) {
                load_program(argv[1]);
            } else {
                printf("Usage: load [program filename]\n");
            }
        } else if (strmatch(argv[0], "run")) {
            if (argc == 1) {
                run_program();
            } else {
                printf("Usage: run\n");
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
        } else {
#ifdef INPUT_ASSEMBLY
            /**
             * You may hook up @translate() from pa1 here to allow assembly code input!
             */
            unsigned int instr = strtoimax(argv[0], NULL, 0);

            if (!instr) {
                instr = translate(argc, argv);
            }
            process_instruction(instr);
#else
            process_instruction(strtoimax(argv[0], NULL, 0));
#endif
        }
    }

    static int __parse_command(char *command, int *nr_tokens, char *tokens[]) {
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

    int main(int argc, char *const argv[]) {
        char command[MAX_COMMAND] = {'\0'};
        FILE *input = stdin;

        if (argc > 1) {
            input = fopen(argv[1], "r");
            if (!input) {
                fprintf(stderr, "No input file %s\n", argv[1]);
                return EXIT_FAILURE;
            }
        }

        if (input == stdin) {
            printf("%s", __color_start);
            printf("*****************************************************\n");
            printf(" Welcome to SCE212 MIPS Termlink v0.3\n");
            printf("\n");
            printf(" SCE212 Model 2023-F is the most reliable client\n");
            printf(" terminal ever developed to run MIPS programs in\n");
            printf(" Vault 212.\n");
            printf("\n");
            printf("- VALID :   November 17 (Fri)\n");
            printf("- MANUAL:   https://git.ajou.ac.kr/sslab/ca-pa2.git\n");
            printf("- SUBMIT:   https://sslab.ajou.ac.kr/pass\n");
            printf("\n");
            printf("- QNA AT AJOUBB ENCOURAGED!!\n");
            printf("\n");
            printf("\n");
            printf("\n");
            printf(">> ");
            printf("%s", __color_end);
        }

        while (fgets(command, sizeof(command), input)) {
            char *tokens[MAX_NR_TOKENS] = {NULL};
            int nr_tokens = 0;

            for (size_t i = 0; i < strlen(command); i++) {
                command[i] = tolower(command[i]);
            }

            if (__parse_command(command, &nr_tokens, tokens) < 0)
                continue;

            __process_command(nr_tokens, tokens);

            if (input == stdin) printf("%s>> %s", __color_start, __color_end);
        }

        if (input != stdin) fclose(input);

        return EXIT_SUCCESS;
    }
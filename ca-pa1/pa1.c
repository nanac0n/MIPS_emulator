/**********************************************************************
 * Copyright (c) 2021-2023
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
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <errno.h>

/* To avoid security error on Visual Studio */
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4996)

/*====================================================================*/
/*          ****** DO NOT MODIFY ANYTHING FROM THIS LINE ******       */
#define MAX_NR_TOKENS	16	/* Maximum length of tokens in a command */
#define MAX_TOKEN_LEN	32	/* Maximum length of single token */
#define MAX_ASSEMBLY	128 /* Maximum length of assembly string */

typedef unsigned char bool;
#define true	1
#define false	0
/*          ****** DO NOT MODIFY ANYTHING UP TO THIS LINE ******      */
/*====================================================================*/


/***********************************************************************
 * translate()
 *
 * DESCRIPTION
 *   Translate assembly represented in @tokens[] into a MIPS instruction.
 *   This translate should support following 13 assembly commands
 *
 *    - add : R
 *    - addi : I
 *    - sub : R
 *    - and : R
 *    - andi : I
 *    - or : R
 *    - ori : I
 *    - nor : R
 *    - lw : I
 *    - sw : I
 *    - sll : S
 *    - srl : S
 *    - sra : S
 *    - beq : I -> addr이 중앙에 들어감
 *    - bne : I
 *
 * RETURN VALUE
 *   Return a 32-bit MIPS instruction
 *
 */

char* MIPS_Registers(char* reg){
	if(!strcmp(reg, "zero"))
		return "00000";
	else if(!strcmp(reg, "at"))
		return "00001";
	else if(!strcmp(reg, "v0"))
		return "00010";
	else if(!strcmp(reg,"v1"))
		return "00011";
	else if(!strcmp(reg, "a0"))
		return "00100";
	else if(!strcmp(reg, "a1"))
		return "00101";
	else if(!strcmp(reg, "a2"))
		return "00110";
	else if(!strcmp(reg, "a3"))
		return "00111";
	else if(!strcmp(reg, "t0"))
		return "01000";
	else if(!strcmp(reg,"t1"))
		return "01001";
	else if(!strcmp(reg, "t2"))
		return "01010";
	else if(!strcmp(reg, "t3"))
		return "01011";
	else if(!strcmp(reg, "t4"))
		return "01100";
	else if(!strcmp(reg, "t5"))
		return "01101";
	else if(!strcmp(reg, "t6"))	
		return "01110";
	else if(!strcmp(reg, "t7"))
		return "01111";
	else if(!strcmp(reg, "s0"))
		return "10000";
	else if(!strcmp(reg, "s1"))
		return "10001";
	else if(!strcmp(reg, "s2"))
		return "10010";
	else if(!strcmp(reg, "s3"))
		return "10011";
	else if(!strcmp(reg, "s4"))
		return "10100";
	else if(!strcmp(reg, "s5"))
		return "10101";
	else if(!strcmp(reg, "s6"))
		return "10110";
	else if(!strcmp(reg, "s7"))
		return "10111";
	else if(!strcmp(reg, "t8"))
		return "11000";
	else if(!strcmp(reg, "t9"))
		return "11001";
	else if(!strcmp(reg, "k1"))
		return "11010";
	else if(!strcmp(reg, "k2"))
		return "11011";
	else if(!strcmp(reg, "gp"))
		return "11100";
	else if(!strcmp(reg, "sp"))
		return "11101";
	else if(!strcmp(reg, "fp"))
		return "11110";
	else if(!strcmp(reg, "ra"))
		return "11111";
	return NULL;
}


static unsigned int translate(int nr_tokens, char *tokens[])
{
	char opcode[9], rs[6], rt[6], rd[100], shamt[100], immed[17],funct[7];
	char result[100] ={0,};
	int result_hex = 0x00000000000000000000000000000000;
	// R-type
	if((!strcmp(tokens[0], "add")||!strcmp(tokens[0], "sub")||!strcmp(tokens[0], "and")||!strcmp(tokens[0], "or")||!strcmp(tokens[0], "nor"))){
		strcpy(opcode,"000000");
		strcpy(shamt, "00000");
		
		strcpy(rs,MIPS_Registers(tokens[2]));
		strcpy(rt, MIPS_Registers(tokens[3]));
		strcpy(rd,MIPS_Registers(tokens[1]));

		if(!strcmp(tokens[0], "add")){
			strcpy(funct,"100000" ) ;
		}
		else if(!strcmp(tokens[0], "sub")){
			strcpy(funct,"100010" ) ;
		}
		else if(!strcmp(tokens[0], "and")){
			strcpy(funct,"100100" ) ;
		}
		else if(!strcmp(tokens[0], "or")){
			strcpy(funct,"100101" ) ;
		}
		else if(!strcmp(tokens[0], "nor")){
			strcpy(funct,"100111" ) ;
		}
		sprintf(result, "%s%s%s%s%s%s", opcode, rs, rt, rd, shamt, funct);
		result_hex = strtol(result, NULL, 2);
		return result_hex;
	}
	// S-type
	else if(!strcmp(tokens[0], "sll")||!strcmp(tokens[0], "srl")||!strcmp(tokens[0], "sra")){
		int shamtInt, bit = 0;
		int count = 0;
		if(!strcmp(tokens[0], "sll"))
			strcpy(funct,"000000");
		if(!strcmp(tokens[0], "srl"))
			strcpy(funct,"000010") ;
		if(!strcmp(tokens[0], "sra"))
			strcpy(funct,"000011") ;

		strcpy(opcode,"000000");
		strcpy(rs, "00000");
		strcpy(rt, MIPS_Registers(tokens[2]));	
		//strcpy(shamt, tokens[3])	

		if(tokens[3][1] == 'x' || tokens[3][0]== '-')
			// printf("this is hex\n");
			shamtInt = strtol(tokens[3], NULL, 16);
		else 
			shamtInt = strtol(tokens[3], NULL, 10);

		; //10진수 문자열
		// 10진수 문자열 -> 10진수 정수 -> 2진수 문자열
		// int shamt_int = strtol(shamt, NULL, 10);
		
		for (int i = 4; i >= 0; i--) {
        	int bit = (shamtInt >> i) & 1;
			sprintf(&shamt[count] ,"%d", bit);
			count++;
    	}
		strcpy(rd, MIPS_Registers(tokens[1]));
		sprintf(result, "%s%s%s%s%s%s", opcode, rs, rt, rd, shamt, funct);

		result_hex = strtol(result, NULL, 2);
		return result_hex;	
	}
	// I-type
	else if(!strcmp(tokens[0], "addi")||!strcmp(tokens[0], "andi")||!strcmp(tokens[0], "ori")||!strcmp(tokens[0], "beq")||!strcmp(tokens[0], "bne")){
		int ImmedVal, bit = 0;
		int count = 0;

		if(!strcmp(tokens[0], "addi"))
			strcpy(opcode,"001000");
		else if(!strcmp(tokens[0], "andi"))
			strcpy(opcode, "001100");
		else if(!strcmp(tokens[0], "ori"))
			strcpy(opcode,"001101");
		else if(!strcmp(tokens[0], "beq"))
			strcpy(opcode,"000100");
		else if(!strcmp(tokens[0], "bne"))
			strcpy(opcode, "000101");

		strcpy(rt,MIPS_Registers(tokens[1]));
		strcpy(rs,MIPS_Registers(tokens[2]));

		if(tokens[3][1] == 'x' || tokens[3][0]== '-'){
			ImmedVal = strtol(tokens[3], NULL, 16); //16진수 정수형
		}
		else 
			ImmedVal = strtol(tokens[3], NULL, 10); //10진수 정수형

		//2진수로 변환
		for (int i = 15; i >= 0; i--) {
			int bit = (ImmedVal >> i) & 1;
			sprintf(&immed[count] ,"%d", bit);
			count++;
		}
		
		sprintf(result, "%s%s%s%s", opcode, rs, rt, immed);
		result_hex = strtol(result, NULL, 2);
		return result_hex;	
	}
	else if(!strcmp(tokens[0], "lw")||!strcmp(tokens[0], "sw")){
		int accountVal, bit, count = 0;
		char address[100] = "00000000000";

		if(!strcmp(tokens[0], "lw"))
				strcpy(opcode,"100011");
			else if(!strcmp(tokens[0], "sw"))
				strcpy(opcode,"101011");

		strcpy(rt,MIPS_Registers(tokens[1]));
		strcpy(rs,MIPS_Registers(tokens[3]));

		if(tokens[2][1] == 'x' || tokens[2][0]== '-'){
			accountVal = strtol(tokens[2], NULL, 16); 
		}

		for (int i = 15; i >= 0; i--) {
			int bit = (accountVal >> i) & 1;
			sprintf(&immed[count] ,"%d", bit);
			count++;
		}

		sprintf(result, "%s%s%s%s", opcode, rs, rt, immed);
		result_hex = strtol(result, NULL, 2);
		return result_hex;	
	}

}

/***********************************************************************
 * parse_command()
 *
 * DESCRIPTION
 *   Parse @assembly, and put each assembly token into @tokens[] and the number
 *   of tokes into @nr_tokens. You may use this implemention or your own
 *   from PA0.
 *
 *   A assembly token is defined as a string without any whitespace (i.e., space
 *   and tab in this programming assignment). For exmaple,
 *     command = "  add t1   t2 s0 "
 *
 *   then, nr_tokens = 4, and tokens is
 *     tokens[0] = "add"
 *     tokens[1] = "t0"
 *     tokens[2] = "t1"
 *     tokens[3] = "s0"
 *
 *   You can assume that the characters in the input string are all lowercase
 *   for testing.
 *
 *
 * RETURN VALUE
 *   Return 0 after filling in @nr_tokens and @tokens[] properly
 *
 */
static int parse_command(char *assembly, int *nr_tokens, char *tokens[])
{
	char *curr = assembly;
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

	return 0;
}


/*====================================================================*/
/*          ****** DO NOT MODIFY ANYTHING BELOW THIS LINE ******      */

/***********************************************************************
 * The main function of this program.
 */
int main(int argc, char * const argv[])
{
	char assembly[MAX_ASSEMBLY] = { '\0' };
	FILE *input = stdin;

	if (argc > 1) {
		input = fopen(argv[1], "r");
		if (!input) {
			fprintf(stderr, "No input file %s\n", argv[0]);
			return EXIT_FAILURE;
		}
	}

	if (input == stdin) {
		printf("*********************************************************\n");
		printf("*          >> SCE212 MIPS translator  v0.10 <<          *\n");
		printf("*                                                       *\n");
		printf("*                                       .---.           *\n");
		printf("*                           .--------.  |___|           *\n");
		printf("*                           |.------.|  |=. |           *\n");
		printf("*                           || >>_  ||  |-- |           *\n");
		printf("*                           |'------'|  |   |           *\n");
		printf("*                           ')______('~~|___|           *\n");
		printf("*                                                       *\n");
		printf("*                                   Fall 2023           *\n");
		printf("*********************************************************\n\n");
		printf(">> ");
	}

	while (fgets(assembly, sizeof(assembly), input)) {
		char *tokens[MAX_NR_TOKENS] = { NULL };
		int nr_tokens = 0;
		unsigned int instruction;

		for (size_t i = 0; i < strlen(assembly); i++) {
			assembly[i] = tolower(assembly[i]);
		}

		if (parse_command(assembly, &nr_tokens, tokens) < 0)
			continue;

		instruction = translate(nr_tokens, tokens);

		fprintf(stderr, "0x%08x\n", instruction);

		if (input == stdin) printf(">> ");
	}

	if (input != stdin) fclose(input);

	return EXIT_SUCCESS;
}

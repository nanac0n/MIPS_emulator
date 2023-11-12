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
#include <errno.h>
#include <ctype.h>


/* To avoid security error on Visual Studio */
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4996)

#define MAX_NR_TOKENS 32	/* Maximum number of tokens in a command */
#define MAX_TOKEN_LEN 64	/* Maximum length of single token */
#define MAX_COMMAND	256		/* Maximum length of command string */

/***********************************************************************
 * int parse_command(command, nr_tokens, tokens[])
 *
 * DESCRIPTION
 *	Parse @command and put each token into @tokens[] and the number of
 *	tokens into @nr_tokens.
 *
 *  A command token is defined as a string without any whitespace (i.e., *space*
 *  and *tab* in this programming assignment) in the middle. For exmaple,
 *    command = "  Hello world   Ajou   University!!  "
 *
 *  then, the command can be split into four command tokens, which are;
 *   tokens[0] = "Hello"
 *   tokens[1] = "world"
 *   tokens[2] = "Ajou"
 *   tokens[3] = "University!!"
 *
 *  Accordingly, nr_tokens should be 4. Another exmaple is;
 *   command = "  add r0   r1 r2 "
 *
 *  then, nr_tokens = 4, and tokens are
 *   tokens[0] = "add"
 *   tokens[1] = "r0"
 *   tokens[2] = "r1"
 *   tokens[3] = "r2"
 *
 *
 * RESTRICTION
 *  DO NOT USE banned functions. You should implement the feature by your own 
 *  if you need some features of the functions. Followings are the list of 
 *  banned functions
 *  - All C library functions starting with 'str'
 *    - strtok, strlen, strcpy, ...
 *    - ...
 *	- All C library functions starting with 'mem'
 *	  - memcpy, memmove, ...
 *	  - ...
 *	- All C library functions ending with 'scanf'
 *	  - scanf, fscanf, sscanf, vscanf, vsscanf, vsscanf
 *	- bcopy
 *
 *
 * RETURN VALUE
 *	Return 0 after filling @nr_tokens and @tokens[] properly
 *
 */
static int parse_command(char *command, int *nr_tokens, char *tokens[])
{ 
	int tok_ctr = 0; 
	for(int ctr = 0; command[ctr] != '\0'; ctr++){ // 한줄씩 읽어옴
		char tokenArray[64]={0,};
		int len_tok = 0;

		for (int i=0; isspace(command[ctr]) == 0; i++) {  // 공백이 아닌 경우
			tokenArray[i] = command[ctr];
			ctr++;
			len_tok = i+1;
		}  	

		if(len_tok>0 && tokenArray[0] != '\0'){ // 토큰이 존재하는 경우, token 대입 
			tokens[tok_ctr] = (char*)malloc(sizeof(char)*(len_tok+1)); // 토큰 길이만큼 메모리 할당
			for(int i=0; i<len_tok; i++){ // 토큰을 tokenArray에서 tokens로 복사
				tokens[tok_ctr][i] = tokenArray[i];
			}
			tokens[tok_ctr][len_tok] = '\0'; // 토큰의 끝에 NULL 대입
			tok_ctr++;
		}
	}
	*nr_tokens = tok_ctr;
	return 0;
}


/***********************************************************************
 * The main function of this program. DO NOT CHANGE THE CODE BELOW
 */
int main(int argc, char * const argv[])
{
	char line[MAX_COMMAND] = { '\0' };
	FILE *input = stdin;

	if (argc == 2) {
		input = fopen(argv[1], "r");
		if (!input) {
			fprintf(stderr, "Unable to open file %s\n", argv[1]);
			return EXIT_FAILURE;
		}
	}

	while (fgets(line, sizeof(line), input)) {
		char *tokens[MAX_NR_TOKENS] = { NULL };
		int nr_tokens;

		parse_command(line, &nr_tokens, tokens);

		fprintf(stderr, "nr_tokens = %d\n", nr_tokens);
		for (int i = 0; i < nr_tokens; i++) {
			fprintf(stderr, "tokens[%d] = %s\n", i, tokens[i]);
		}
		fprintf(stderr, "\n");
	}

	if (input != stdin) fclose(input);

	return EXIT_SUCCESS;
}

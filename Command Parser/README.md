## Project #0: Command Parser

### Goal

Implement a simple string parser to get familiar with (PASS)[https://sslab.ajou.ac.kr/pass] system.


### Problem Specification

We will implement a MIPS interpreter soon (no worries, you will get to know what it means soon). To that end, you should first make a simple program that understands user inputs and commands.

In this project, your task is to **split an input strings into command tokens**. By the C standard, a string is defined as a sequence of characters which is terminated with `\0`. Command tokens are the strings that are obtained by splitting the input string with delimitors, which are whitespace characters in this PA. Any heading and trailing whitespaces should be also removed from the command tokens. Multiple whitespaces in a row can be considered as a single whitespace.

For example, let's think about an input string " Hello   world ". The command tokes are "Hello" and "world". **Note that the input string is split into tokens delimited by whitespaces (' ' in this example), and the tokens do not contain any whitespaces**.

Another example string is " add   r1  r2   r3   ". It should be tokenized into "add", "r1", "r2", and "r3".

The required functionality must be implemented in `parse_command()` function. The user input is passed through `command`. After parsing `command` into tokens, the tokens should be assigned to `tokens[]` in order, which is passed as the function argument. Also, the number of tokens should be assigined to `*nr_tokens` which will be used by the framework.

```c
char *command   --> "  add   r0   r1 r2  "

*nr_tokens = 4
tokens[0]  -->  "add"
tokens[1]  -->  "r0"
tokens[2]  -->  "r1"
tokens[3]  -->  "r2"
tokens[4]... =  NULL
```

### Restrictions

- You **should not use any string manipulation functions from any libraries**. Followings are the banned functions;
  - All C library functions starting with `str` (e.g., `strtok`, `strlen`, `strcpy`, ...)
  - All C library functions starting with `mem` (e.g., `memcpy`, `memmove`, ...)
  - All C library functions ending with `scanf` (e.g., `scanf`, `fscanf`, ...)
  - `bcopy`
This implies that you should implement your own string manipulation functions if you need it. **You will get 0 point if you use any of them**. Note that `malloc()` and `free()` are not string manipulation functions so you can use it if needed. If unsure, question to the instructor through AjouBb.
- Do not change or redefine `parse_command()` and `main()` functions; use the functions and their arguments as-is.
- Use `isspace()` C library function to check whether a character can be assumed as a whitespace.
- Printing messages to stand output (e.g., `printf()`) is OK, but do not `fprintf(stderr ...)` since the grading system evaluates your work with the output to the `stderr`.

- For all PAs (including this and next ones), the restriction is applied like a blacklist (c.f., whitelist); **you can do whatever you want to do as long as it is not explicitly restricted**. For example, you can include as many header files as you want since it is not explicitly prohibited. Also, you can define your own custom functions in addition to the provided ones.
- Check the default restrictions explained in the tutorial session.


### Logistics

- This is an individual project; you work on the assignement alone.
- You can use **up to 3 slip tokens throughout this semester**.
  - The instructor will not answer to questions during the grace period.
- All submission and grading are done automatically through (PASS)[https://sslab.ajou.ac.kr/pass]. Please follow the instructions explained in the class.
	- Submit `pa0.c` file. Make sure that you turn in the right file with the designated file name. (90 pts)
	- Explain your parsing strategy on a **1-page PDF document**. (10 pts)
  - No need to turn in the git repository; ignore git in this PA.
- `input-simple` and `input-adv` files in the code template contain the input sequences for the grading. They contain whitespaces in special characters which are usually not properly handled when you copy-paste the contents. Thus, specify the file name as the input instead of copy-pasting it for testing (read `main()` how you can pass it as the input).


### Tips and Notes

- Briefly speaking, the overall implementation might be as follows; start scanning from the beginning of `command`, skipping all whitespaces until meet a non-whitespace character. That will be the beginning of the token. Then, continue scanning until meet any whitespace character. That will be the end of the token. Repeat this until entire `command` is scanned. Assign the starting position of each token into `tokens[]` and count `nr_tokens` accordingly.
- Read comments in the skeleton code carefully.
- Make sure every string is terminated with '\0'.
- Post freely on QnA board on AjouBb to question about the project.

Good luck and have fun!

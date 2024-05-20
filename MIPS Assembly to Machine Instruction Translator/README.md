## Project #1: MIPS Assembly to Machine Instruction Translator

### ***Due on October 13 (Friday), 24:00 ***

### Goal

Translate MIPS assembly code into corresponding MIPS machine instructions.


### Problem Specification

- Implement a MIPS assembly translator (a.k.a assembler) that translates MIPS assembly into MIPS machine code one line at a time. Your implementation translates tokens into bits, and merges them to produce one 32-bit machine instruction.


- The framework does some work for you: it gets a line of input from CLI, makes it to lowercase, and calls `parse_command()`. `parse_command()` splits the line into tokens as of PA0. Finally, the framework will call `translate()` function with the tokens.

- Write your code in `translate()` to translate the tokenized assembly into a 32-bit MIPS machine instruction.

- The translator should support following MIPS assembly instructions. Look up the MIPS reference sheet to find the format for each instruction.

  | Name   | Opcode / opcode + funct |
  | ------ | ----------------------- |
  | `add`  | 0 + 0x20                |
  | `addi` | 0x08                    |
  | `sub`  | 0 + 0x22                |
  | `and`  | 0 + 0x24                |
  | `andi` | 0x0c                    |
  | `or`   | 0 + 0x25                |
  | `ori`  | 0x0d                    |
  | `nor`  | 0 + 0x27                |
  | `sll`  | 0 + 0x00                |
  | `srl`  | 0 + 0x02                |
  | `sra`  | 0 + 0x03                |
  | `lw`   | 0x23                    |
  | `sw`   | 0x2b                    |
  | `beq`  | 0x04                    |
  | `bne`  | 0x05                    |

- We will use a slightly different assembly for ease of implementation.

- The input for R-format instructions will be as follows;
  ```
  add s0 t1 gp    /* s0 <- t1 + gp */
  sub s4 s1 zero  /* s4 <- s1 + zero */
  sll s0 s2 3     /* s0 <- s2 << 3. shift amount comes last */
  sra s1 t0 5     /* s1 <- t0 >> 5 with sign extension */
  ```

- I-format instructions are the similar. Except for `lw` and `sw`, immediate values come to the last. For `lw` and `sw` the constant offset is followed by the base address.
  ```
  addi s1 s2 0x16 /* s1 <- s2 + 0x16s2. Immediate values come to the last */
  lw t0 32 s1      /* Load t0 with a word at 32(s1) (s1 + 32) */
  sw t0 32 s1      /* Store value in t0 at 32(s1) */
  beq k1 a2 -4    /* Jump to the 4 previous instruction if k1 and a2
                     registers contain the same value */
  ```

- The 32 MIPS registers are specified in the assembly as follow;

  | Name   | Number |
  | ------ | ------ |
  | zero   | 0      |
  | at     | 1      |
  | v0-v1  | 2-3    |
  | a0-a3  | 4-7    |
  | t0-t7  | 8-15   |
  | s0-s7  | 16-23  |
  | t8-t9  | 24-25  |
  | k1-k2  | 26-27  |
  | gp     | 28     |
  | sp     | 29     |
  | fp     | 30     |
  | ra     | 31     |

- `shamt` and immediate values are inputted as either (1) decimal numbers without any prefix or (2) hexadecimal numbers with `0x` as its prefix. Followings are the examples. Check the Hints/Tips section.
  ```
  10    /* 10 */
  -22   /* -22 */
  0x1d  /* 29 */
  -0x42 /* -66 */
  ```

- Unspecified registers and `shamt` parts should be all 0's. For example, the `shamt` part in `add` instruction should be 0b00000. Likewise, `rs` part should be 0b00000 in `sll` instruction.


### Example
```
...
>> add t0 t1 t2
0x012a4020
>> addi sp sp -0x10
0x23bdfff0
>> sll t0 t1 10
0x00094280
```

### Restrictions

- DO NOT ADD/MODIFY/REMOVE CODES IN THE SPECIFIED ZONES.

- You can submit up to 30 times for this PA.


### Hints/Tips

- You can use any standard C library functions such as `strlen` and `strcpy`. However, Windows-specific functions are banned and it will make a compile error on the server.

- *Bit masking* might be your saver. Learn the concept through Internet search and try to leverage them in your implementation. You will find bit shifting and bitwise AND/OR are very useful and make this PA easy. Of course, you don't have to use them if you don't want to.

- The C syntax allows to use `0b` and `0x` prefix to directly specify a binary/hexadecimal number.
  - `int x = 0b1010 + 0xdead0011;`

- Helpful functions:
  - `strcmp/strncmp`: For matching commands and register names
  - `strtol/strtoimax`: Converting decimal/hexadecimal numbers (regardless of sign) in string to corresponding long/int numbers

- Remember that computers do not store a value with an associated numeral system.


### Submission / Grading

- Use [PASS](https://sslab.ajou.ac.kr/pass) for submission
  - 200 pts in total

- Source: pa1.c (170 pts)
  - Will be tested with the testcase files under the `testcases` directory.
  - Have a look at `Makefile` for testing your implementation with these testcase files.

- Document: *One PDF* document (30 pts)
  - Must include all the followings;
    - How do you translate the instructions
      - How tokens and immediate values are translated, and combined into machine instruction.
      - ***Must include an example case of negative immediate values***.
    - How do you translate the register names to corresponding register numbers
    - ***DO NOT COPY-PASTE OR SCREENSHOT OF CODE***
      - Do not literally read C code nor paste a screenshot of it. If you cannot explain your implementation without showing it, it means that something is not right in the way of your code writing or code explanation.
    - ***Lesson learned***: What you've leared while doing this PA. Do not mention common facts that are discussed in the class.
  - NO MORE THAN ***THREE*** PAGES

- WILL NOT ANSWER THE QUESTIONS ABOUT THOSE ALREADY SPECIFIED ON THE HANDOUT.

# CF - a ColorForth and Tachyon inspired system

<img src="/images/editing.jpg" width="400" height="300" />

## ColorForth influences on CF
- cf is inspired by ColorForth, but it is NOT ColorForth.
- cf has 4 states: DEFINE, COMPILE, INTERPRET, and COMMENT.
- cf uses markers in the source whitespace to control its state.
- cf also supports using the standard words to change the state.
- cf has `a` and `b` registers like ColorForth. It also has a `t` register.
- Unlike ColorForth, they are actually stacks. See below for more information.

## Whitespace characters and WORDS that change the state:

| Byte  | Word | New State |
| :--   | :--  | :--       |
| $01   | ":"  | DEFINE    |
| $02   | "]"  | COMPILE   |
| $03   | "["  | INTERPRET |
| $04   |      | COMMENT   |
|       | ";"  | INTERPRET |

## Notes:
- DEFINE changes the state to COMPILE after adding the word to the dictionary.
- ";" compiles EXIT and changes the state to INTERPRET.
- The `(` word simply skips words until `)`. It does not change the state.
- CF still supports IMMEDIATE words.
- Flow control words like IF/THEN would be marked as IMMEDIATE.
- They are defined in the source file.

## Tachyon's influence on cf
In cf, a program is a sequence of OPCODEs.<br/>
An OPCODE is an unsigned CELL (32 or 64 bits).<br/>
Primitives are assigned sequentially from 0 to `(bye)`.<br/>
If an OPCODE is less than or equal to `(bye)`, it is a primitive.<br/>
Else if the top 3 bits are set, it is a literal with the top 3 bits masked off.<br/>
Else, it is the CODE slot (XT) of a word to execute.<br/>
CODE slots 0-24 are used by cf. Slots 25-`(bye)` are unused by CF.<br/>
They are free CELL-sized slots that can be used for any purpose.<br/>
HERE starts at `(bye)+1`.<br/>

## The A, B and T register stacks
In ColorForth, 'a', 'b' and 't' are registers. In CF, they are stacks.<br/>
They have operations similar to those for the return stack (>r, r@, r>). <br/>
The operations for the 'a' stack are: >a, a@, a>, a!, a@+, and a@-.<br/>
The operations for the 'b' and 't' stacks are the same.<br/>

## Architecture
CF is really just a Forth VM, upon which any Forth system can be built.<br/>
To that end, cf defines a set of primitives and the inner/outer interpreters.<br/>
The list of primitives and their definitions is detailed below.<br/>
The rest of the system is defined by the source code file.<br/>
CF takes a source file name as its only argument.<br/>
If cf is executed without arguments, the source file defaults to `BOOT_FN1`.<br/>
If `BOOT_FN1` (cf-boot.fth) is not found, it tries `BOOT_FN2`.<br/>
`BOOT_FN1` and `BOOT_FN2` are defined in cf.h. Change them as appropriate.<br/>
CF provides a single chunk of memory (see cf.h, MEM_SZ) for data and code.<br/>
The CODE area starts at the beginning of the memory.<br/>
The start of the data area (VHERE) is defined in the source file.<br/>

## Embedding cf into a C program
CF can easily be embedded into a C program.<br/>
The cf VM is implemented in `cf.c`.<br/>
The configuration settings and API for cf are located in `cf.h`.<br/>
File `system.c` embeds the CF VM into an executable.<br/>

## Building cf
Building cf is simple and fast since there are only 3 small source files.<br/>
32-bit or 64-bit builds are supported.<br/>

Windows
- There is a `cf.sln` file for Visual Studio.

Linux, OpenBSD, and FreeBSD
- There is a makefile, which uses the system C compiler (specified by the CC variable).
- Or you can easily build cf from the command line:
- Example:

```
# the default is 64 bit cells:
make

# for 32 bit cells:
ARCH=32 make

# or manually:

$CC -m32 -O3 -o cf *.c
$CC -m64 -O3 -o cf *.c
```

## Built-in words
| Word        | Stack Effect | Description |
| :---------- | :----------- | :---------- |
| version     | (--n) | CF version number |
| cell        | (--n) | Size of a CELL in bytes |
| (ha)        | (--n) | Address of HERE |
| (la)        | (--n) | Address of LAST |
| (vha)       | (--n) | Address of VHERE |
| base        | (--n) | Address of BASE |
| state       | (--n) | Address of STATE |
| >in         | (--n) | Address of the input pointer |
| memory      | (--n) | Address of the memory area |
| mem-sz      | (--n) | Size of the memory area |
| (lit)       | (--n) | Opcode for LIT |
| (jmp)       | (--n) | Opcode for JMP |
| (jmpz)      | (--n) | Opcode for JMPZ |
| (jmpnz)     | (--n) | Opcode for JMPNZ |
| (njmpz)     | (--n) | Opcode for NJMPZ |
| (njmpnz)    | (--n) | Opcode for NJMPNZ |
| (exit)      | (--n) | Opcode for EXIT |
| (ztype)     | (--n) | Opcode for ZTYPE |
| (bye)       | (--n) | Opcode for BYE |
| de-sz       | (--n) | Size of a dictionary entry |
| stk-sz      | (--n) | Size of the Data and Return stacks |
| lstk-sz     | (--n) | Size of the Loop stack |
| tstk-sz     | (--n) | Size of the A, B, and T stacks |
| (dsp)       | (--n) | Address of of the Data stack pointer |
| (rsp)       | (--n) | Address of of the Return stack pointer |
| (lsp)       | (--n) | Address of of the Loop stack pointer |
| (asp)       | (--n) | Address of of the A stack pointer |
| (bsp)       | (--n) | Address of of the B stack pointer |
| (tsp)       | (--n) | Address of of the T stack pointer |
| dstk        | (--n) | Address of the Data stack |
| rstk        | (--n) | Address of the Return stack |
| lstk        | (--n) | Address of the Loop stack |
| astk        | (--n) | Address of the A stack |
| bstk        | (--n) | Address of the B stack |
| tstk        | (--n) | Address of the T stack |
| (output-fp) | (--n) | Address of the output file handle. 0 means STDOUT |

## Primitives Reference

The CF primitives defined in the `PRIMS` macro in `cf.c`.

### Stack Operations
| Word | Stack Effect | Description |
| :--- | :----------- | :---------- |
| `dup` | (n--n n) | Duplicate top of stack (TOS) |
| `swap` | (a b--b a) | Swap top two stack items |
| `drop` | (n--) | Remove top of stack |
| `over` | (a b--a b a) | Copy second item to top |

### Flow Control and Literal
| Prim | Stack  | Description |
| :--- | :----  | :---------- |
| (lit)    | (--n)  | **LIT**: Push CODE[PC]. Increment PC. |
| (jmp)    | (--)   | **JMP**: PC = CODE[PC]. |
| (jmpz)   | (f--)  | **JMPZ**: If (pop() == 0) then PC = CODE[PC], else increment PC. |
| (jmpnz)  | (f--)  | **JMPNZ**: If (pop() != 0) then PC = CODE[PC], else increment PC. |
| (njmpz)  | (f--f) | **NJMPZ**: If (TOS == 0) then PC = CODE[PC], else increment PC. |
| (njmpnz) | (f--f) | **NJMPNZ**: If (TOS != 0) then PC = CODE[PC], else increment PC. |

### Memory Access
| Word | Stack Effect | Description |
| :--- | :----------- | :---------- |
| `@`  | (addr--n) | Fetch cell from address |
| `!`  | (n addr--) | Store cell to address |
| `c@` | (addr--c) | Fetch byte from address |
| `c!` | (c addr--) | Store byte to address |
| `+!` | (n addr--) | Add n to cell at address |

### Arithmetic
| Word | Stack Effect | Description |
| :--- | :----------- | :---------- |
| `+`  | (a b--sum) | Add two numbers |
| `-`  | (a b--diff) | Subtract (a-b) |
| `*`  | (a b--prod) | Multiply two numbers |
| `/`  | (a b--quot) | Divide (a/b) |
| `/mod` | (a b--rem quot) | Divide, return remainder and quotient |
| `1+` | (n--n+1) | Increment TOS by 1 |
| `1-` | (n--n-1) | Decrement TOS by 1 |

### Comparison
| Word | Stack Effect | Description |
| :--- | :----------- | :---------- |
| `<`  | (a b--flag) | Less than comparison |
| `=`  | (a b--flag) | Equality comparison |
| `>`  | (a b--flag) | Greater than comparison |
| `0=` | (n--flag) | Test if zero |

### Logical Operations
| Word  | Stack Effect | Description |
| :---  | :----------- | :---------- |
| `and` | (a b--n) | Bitwise AND |
| `or`  | (a b--n) | Bitwise OR |
| `xor` | (a b--n) | Bitwise XOR |
| `com` | (n--~n) | Bitwise complement |

### Control Flow
| Word   | Stack Effect | Description |
| :----- | :----------- | :---------- |
| `exit` | (--) (R: n--) | Exit current word |
| `for`  | (count--) | Begin counted loop, starting at 0. |
| `i`    | (--I) | Current loop index `I` |
| `next` | (--) | Increment `I`. Restart loop if (I < count) |

### Return Stack
| Word | Stack Effect | Description |
| :--- | :----------- | :---------- |
| `>r` | (n--) (R:--n) | Move n to return stack |
| `r@` | (--n) (R: n--n) | Copy n from return stack |
| `r>` | (--n) (R: n--) | Move n from return stack |
| `rdrop` | (--) (R: n--) | Drop from return stack |

### A Stack
| Word  | Stack Effect | Description |
| :---- | :----------- | :---------- |
| `>a`  | (n--) (A:--n) | Move/push n to the A stack |
| `a!`  | (n--) | Store n to A-TOS |
| `a@`  | (--n) (A: n--n) | Copy n from A-TOS |
| `a@+` | (--n) (A: n--n+1) | Copy A-TOS, then increment A-TOS |
| `a@-` | (--n) (A: n--n-1) | Copy A-TOS, then decrement A-TOS |
| `a>`  | (--n) (A: n--) | Move/pop n from A |

### B Stack
| Word  | Stack Effect | Description |
| :---- | :----------- | :---------- |
| `>b`  | (n--) (B:--n) | Move/push n to the B stack |
| `b!`  | (n--) | Store n to B-TOS |
| `b@`  | (--n) (B: n--n) | Copy n from B-TOS |
| `b@+` | (--n) (B: n--n+1) | Copy B-TOS, then increment B-TOS |
| `b@-` | (--n) (B: n--n-1) | Copy B-TOS, then decrement B-TOS |
| `b>`  | (--n) (B: n--) | Move/pop from B stack |

### T Stack
| Word  | Stack Effect | Description |
| :---- | :----------- | :---------- |
| `>t`  | (n--) (T:--n) | Move/push n to the T stack |
| `t!`  | (n--) | Store n to T-TOS |
| `t@`  | (--n) (T: n--n) | Copy n from T-TOS |
| `t@+` | (--n) (T: n--n+1) | Copy T-TOS, then increment T-TOS |
| `t@-` | (--n) (T: n--n-1) | Copy T-TOS, then decrement T-TOS |
| `t>`  | (--n) (T: n--) | Move/pop n from T stack |

### I/O Operations
| Word | Stack Effect | Description |
| :--- | :----------- | :---------- |
| `emit`  | (c--) | Output character |
| `key`   | (--c) | Wait for and read a character |
| `key?`  | (--f) | f: 1 if a key was pressed, 0 otherwise |
| `ztype` | (addr--) | Print null-terminated string at addr |

### File Operations
| Word | Stack Effect | Description |
| :--- | :----------- | :---------- |
| `fopen`  | (name mode--fh) | Open file, return file handle fh |
| `fclose` | (fh--) | Close file fh |
| `fread`  | (buf sz fh--n) | Read from file, return bytes read |
| `fwrite` | (buf sz fh--n) | Write to file, return bytes written |
| `fseek`  | (fh offset--) | Seek to position in file (uses SEEK_SET) |

### String Operations
| Word | Stack Effect | Description |
| :--- | :----------- | :---------- |
| `s-cpy` | (dest src--dst) | Copy null-terminated string |
| `s-eqi` | (s1 s2--flag) | Case-insensitive string comparison |
| `s-len` | (addr--len) | Get string length |
| `cmove` | (src dest n--) | Move n bytes from src to dest |

### System Operations
| Word | Stack Effect | Description |
| :--- | :----------- | :---------- |
| `lit,` | (n--) | Compile literal value |
| `outer` | (addr--) | Execute Forth source at addr |
| `addword` | (--) | Add word to dictionary (uses >in) |
| `find` | (--xt addr) | Find word (uses >in), return xt and dict addr |
| `timer` | (--n) | Get current time |
| `ms` | (n--) | Sleep for n milliseconds |
| `system` | (cmd--) | Execute system(cmd) |
| `bye` | (--) | Exit CF |

## Adding a primitive to CF

Adding a primitive to CF is easy. Add an X() line to the PRIMS macro.<br/>
The embedded X() macro in PRIMS is a powerful use of C macros.<br/>
The X(op, name, code) macro takes 3 parameters:
- A name for the ENUM of opcodes (used by the inner interpreter)
- A name for the word to be created in the Forth dictionary entry
- Code that implements the primitive's action. This can be a call to any function.

The `PRIMS` macro is used to create the opcode values and code for the switch statement in `cfInner()`, and to create the dictionary entries in `cfInit()`.

For example, in the following example, `SCOPY` is the name for the enum, `s-cpy` is the name for the Forth dictionary entry, and `t=pop(); strcpy((char*)TOS, (char*)t);` is the code to be executed when `SCOPY` is encountered by the inner interpreter.
```c
	X(SCOPY,   "s-cpy",   t=pop(); strcpy((char*)TOS, (char*)t); ) \
```

## Blocks
- Blocks are not native to cf.
- They are implemented in the default source file.
- In this implentation, they are 2048 bytes.
- Feel free to change it in any way that suits your needs.

## The Editor
- An editor is not native to cf.
- One is implemented in the default source file.
- The editor has a vi-like feel. See `ed-cases` in cf-boot.fth for details.
- It uses 23 rows and 89 columns, for a total of 2047 editable bytes.
- The last byte is set to 0 to NULL terminate the block.
- This stops the OUTER interpreter.

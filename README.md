# CF - a ColorForth and Tachyon inspired system

<img src="/images/editing.jpg" width="400" height="300" />

## ColorForth influences on CF
- NOTE: **cf is a work in progress. Please collaborate with me on it!**
- cf is inspired by ColorForth, but it is NOT ColorForth.
- cf has 4 states: DEFINE, INTERPRET, COMPILE, and COMMENT.
- cf uses markers in the source to control its state.
- cf has `a` and `b` registers like ColorForth. It also has a `t` register.
- cf supports using either a byte in the whitespace or a word to change the state.

## Whitespace characters and WORDS that change the state:

| Byte  | Word(s)     | New State |
| :--   | :--         | :--       |
| $01   | ":"         | DEFINE    |
| $02   | "]" or ")"  | COMPILE   |
| $03   | "[" or "))" | INTERPRET |
| $04   | "(" or "((" | COMMENT   |
|       | ";"         | INTERPRET |

## Notes:
- DEFINE changes the state to COMPILE after adding the word to the dictionary.
- ";" compiles EXIT and changes the state to INTERPRET.
- There is no difference between `(` and `((`, they make the code more readable.
- CF still supports IMMEDIATE words.
- Flow control words like IF/THEN would be marked as IMMEDIATE.
- They are defined in the source file.

```
(( A comment in INTERPRET mode ))
: hello ( A comment in COMPILE mode ) ." Hello World!" ;
hello
```

## Tachyon's influence on cf
In cf, a program is a sequence of OPCODEs.<br/>
An OPCODE is a CELL-sized unsigned number (32 or 64 bits).<br/>
Primitives are assigned numbers sequentially from 0 to `(bye)`.<br/>
If an OPCODE is less than or equal to `(bye)`, it is a primitive.<br/>
If the top 3 bits are set, it is an unsigned literal with the top 3 bits masked off.<br/>
Else, it is the CODE slot of a word to execute.<br/>
CODE slots 0-24 are used by cf.<br/>
CODE slots 25-`(bye)` are free CELL-sized slots that can be used for any purpose.<br/>
HERE starts at `(bye)+1`.<br/>

## Architecture
CF is really just a Forth VM, upon which any Forth system can be built.<br/>
To that end, cf provides a set of primitives and the inner/outer interpreters.<br/>
See `cf.c` for the list of primitives.
The rest of the system is defined by the source code file.<br/>
CF takes a source file as its only argument.<br/>
If cf is executed without arguments, the default source file is 'boot.fth'.<br/>
CF provides a single chunk of memory (see cf.h, MEM_SZ) for data and code.<br/>
The CODE area starts at the beginning of the memory.<br/>

## Embedding cf into a C program
CF can easily be embedded into a C program.<br/>
The cf VM is implemented in `cf.c`.<br/>
The configuration and API for cf are located in `cf.h`.<br/>
That is what `system.c` does, it creates a C program around the cf VM.<br/>

## Building cf
Building cf is simple and fast since there are only 3 small source files.<br/>

Windows
- There is a `cf.sln` file for Visual Studio. 32-bit or 64-bit builds are supported.

Linux, OpenBSD, and FreeBSD
- There is a makefile, which uses the system C compiler (specified by the CC variable).
- Or you can easily build cf from the command line:
- Example:

```
# default, 64 bit:
make

# for 32 bit:
ARCH=32 make

# or manually:

$CC -m32 -O3 -o cf *.c
$CC -m64 -O3 -o cf *.c
```

## Blocks
- Blocks are not native to cf.
- They are implemented in the default source file.

## The Editor
- A block editor is implemented in the default source file.

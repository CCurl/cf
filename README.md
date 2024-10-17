# cf - a ColorForth inspired system

<img src="/images/editing.jpg" width="400" height="300" />

## What is cf?
- NOTE: **cf is a work in progress. Please collaborate with me on it!**
- cf is inspired by ColorForth, but it is NOT ColorForth.
- cf has 4 states: DEFINE, INTERPRET, COMPILE, and COMMENT.
- Like ColorForth, cf uses markers in the source to control its state.
- cf supports using either a byte in the whitespace or a word to change the state.
- cf builds on the VM from c5. See this for details: https://github.com/CCurl/c5

## Whitespace characters and WORDS that change the state:

| Byte  | Word(s)     | New State | Editor Key |
| :--   | :--         | :--       | :-- |
| $01   | ":"         | DEFINE    | F1 |
| $02   | "]" or ")"  | COMPILE   | F2 |
| $03   | "[" or "))" | INTERPRET | F3 |
| $04   | "(" or "((" | COMMENT   | F4 |

## Notes:
- DEFINE changes the state to COMPILE after adding the word to the dictionary
- `;` does NOT change the state to INTERPRET
- There is no difference between `(` and `((`, they make the code more readable
- cf still supports IMMEDIATE words
- Flow control words like IF/THEN are IMMEDIATE
- They are defined in the source file.

```
(( A comment in INTERPRET mode ))
: hello ( A comment in COMPILE mode ) ." hi" ;
[ hello
```

## Architecture
CF is really just the barebones of a Forth core, upon which any Forth system can be created.

To that end, cf provides a set of primitives and the inner/outer interpreters.

The rest of the system is defined by the source Forth code file, provided as an argument when cf is executed.

If cf is executed without arguments, the default source file is 'boot.cf'.

## Building cf
Building cf is simple and fast since there are only 2 small source files.

For Windows, there is a `cf.sln` file for Visual Studio.

For Linux, OpenBSD, and FreeBSD, there is a makefile, which uses the system C compiler (specified by the CC variable). Example:

```
# default, 64 bit:
make

or

# for 64 bit:
ARCH=32 make
```

Or you can easily build it from the command line:

```
gcc -m64 -O3 -o cf *.c

or

clang -m64 -O3 -o cf *.c
```

## Blocks
- Blocks are not native to cf.
- They are implemented in the default source file.

## The Editor
- A simple block editor is implemented in the default source file.

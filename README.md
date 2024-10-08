# cf - a ColorForth inspired system

<img src="/images/editing.jpg" width="400" height="300" />

## What is cf?
- cf is inspired by ColorForth, but it is NOT ColorForth.
- Like ColorForth, cf uses markers in the source to control its state.
- cf builds on the VM from c5. See this for details: https://github.com/CCurl/c5
- cf has 4 states: DEFINE, INTERPRET, COMPILE, and COMMENT.
- cf supports using either a character in the whitespace or a word to change the state.
- cf is a work in progress. **Please collaborate with me on it!**

## Whitespace characters and that change the state:

| Char  | Word(s)     | New State |
| :--   | :--         | :--       |
| 1     | ":"         | DEFINE    |
| 2     | "[" or "))" | INTERPRET |
| 3     | "]" or ")"  | COMPILE   |
| 4     | "(" or "((" | COMMENT   |

## Notes:
- DEFINE changes the state to COMPILE after adding the word to the dictionary
- `;` does NOT change the state to INTERPRET
- There is no difference between `(` and `((`, they make the code more readable
- `)` is simply an alias for `]`, and makes the code more readable
- `))` is simply an alias for `[`, and makes the code more readable
- cf still supports IMMEDIATE words
- Flow control words like IF/THEN are IMMEDIATE
- They and are defined in the source file.

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

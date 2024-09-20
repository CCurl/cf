# cf - a ColorForth inspired system

<img src="/images/editing.jpg" width="400" height="300" />

## What is cf?
- cf is inspired by ColorForth, but it is NOT ColorForth.
- Like ColorForth, cf uses markers in the source to control its mode.
- cf builds on the VM from c5. See this for details: https://github.com/CCurl/c5
- cf has 4 modes: DEFINE, INTERPRET, COMPILE, and COMMENT.
- cf supports using either a marker byte in the whitespace or a word to identify each mode.
- cf is a work in progress. **Please collaborate with me on it!**

## cf architecture
CF is really just the barebones of a Forth core, upon which any system can be created.

To that end, cf provides a set of primitives and the inner/outer interpreters.

The rest of the system is defined by the source Forth code file, provided as an argument when cf is executed.

If cf is executed without arguments, the default source file is 'boot.cf'.

## Building c5
Building c5 is simple and fast since there are only 2 small source files.

For Windows, there is a `c5.sln` file for Visual Studio.

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
gcc -m64 -O3 -o c5 *.c

or

clang -m64 -O3 -o c5 *.c
```

## Notes:
- Flow control words like IF/THEN and are defined in the source file.

## Words and whitespace characters that change the mode:

| Mode      | Char  | Word(s) |
| :--       | :--   | :--     |
| DEFINE    | 1     | ":"     |
| INTERPRET | 2     | "[" or "))" |
| COMPILE   | 3     | "]" or ")"  |
| COMMENT   | 4     | "(" or "((" |

**NOTES:**
- DEFINE changes the mode to COMPILE after adding the word to the dictionary
- cf still supports IMMEDIATE words

## Blocks
- Blocks are not native to cf.
- They are implemented in the default source file.

## The Editor

- The editor is implemented in the default source file.

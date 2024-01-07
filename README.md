# cf - a ColorForth inspired system

![Editing](/images/editing.jpg)

## What is cf?
- cf is inspired by ColorForth, but it is NOT ColorForth.
- Like ColorForth, cf uses markers in the source to control its mode.
- cf uses the VM from c3. See this for details: https://github.com/CCurl/c3
- cf has 6 modes: DEFINE, COMMENT, INLINE, COMPILE, INTERPRET, and MACHINE.
- cf supports using a marker byte in the whitespace or a word to identify each mode.
- cf includes a simple block editor that colors the file to identify how it will be processed.
- cf is a work in progress. Please collaborate with me on it!

## Notes:
- Words like IF/THEN are defined in block 0.
- They are WHITE (INTERPRETED) because they generate code.
- BEGIN, WHILE, UNTIL, and AGAIN are also interpreted.

## The markers:

|Mode       |Color   | Code  |Editor  | Word |
| :--       | :--    | :--   | :--    | :--  |
| DEFINE    | red    | 1     | ctrl-a | ::   |
| COMMENT   | green  | 2     | ctrl-b | ((   |
| INLINE    | orange | 3     | ctrl-c | :I   |
| UNUSED    | blue   | 4     | ctrl-d |      |
| MACHINE   | purple | 5     | ctrl-e | :M   |
| COMPILE   | cyan   | 6     | ctrl-f | [[   |
| INTERPRET | white  | 7     | ctrl-g | ]]   |

## Blocks
- In cf, a block is a file with the name "block-NNN.cf". It has a maximum of BLOCK_SZ bytes.
- BLOCK_SZ, in block.h, is set to 2000. It is easy to change as needed.

## The Editor
Talk about the editor here.

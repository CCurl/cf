# cf - a ColorForth inspired system

## What is cf?
- cf is NOT ColorForth, but it does use markers in the source to control its mode.
- cf is a byte-coded system written in C.
- cf uses the VM from c3. See this for details: https://github.com/CCurl/c3
- cf has 6 modes: DEFINE, COMMENT, INLINE, COMPILE, MACHINE, and INTERPRET.
- cf uses a either a marker code (in the whitespace) or a word to identify each mode.
- cf includes a simple text editor that will color the code to identify how it will be processed.
- cf is a work in progress.

### The markers:

|Mode       |Color   | Code  |Editor  | Word |
| :--       | :--    | :--   | :--    | :--  |
| DEFINE    | red    | 1     | ctrl-a | ::   |
| COMMENT   | green  | 2     | ctrl-b | ((   |
| INLINE    | orange | 3     | ctrl-c | :I   |
| UNUSED    | blue   | 4     | ctrl-d |      |
| MACHINE   | purple | 5     | ctrl-e | :M   |
| COMPILE   | cyan   | 6     | ctrl-f | [[   |
| INTERPRET | white  | 7     | ctrl-g | ]]   |

## Notes:
- Words like IF/THEN and BEGIN/WHILE are defined in block 0.
- They are WHITE (INTERPRETED) because they generate code.
- BEGIN, WHILE, AGAIN, and REPEAT are also interpreted.

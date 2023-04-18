# cf - a ColorForth inspired system

## What is cf?
- cf is a byte-coded system written in C.
- cf is NOT ColorForth, but it does use markers in the source to control its mode.
- cf has 4 modes: COMMENT, DEFINE, COMPILE, ASM, and INTERPRET.
- cf uses a marker to identify each mode.
- Each marker is human-readable, so cf code can also be edited using a "standard" text editor.
- cf includes a simple text editor that will color the code to identify how it will be processed.
- Each marker/mode is given a color in the editor:
    - COMMENT: white
    - DEFINE: red
    - COMPILE: cyan
    - IMMEDIATE: yellow
    - ASM: green
- A marker, by itself or as the first character of a word, puts cf in that mode.
- The markers are as follows:
    - COMMENT: '('
    - DEFINE: ':'
    - COMPILE: '^'
    - IMMEDIATE: '['
    - ASM: '~'

## Notes:
- Words like IF/THEN and BEGIN/WHILE are defined in block 0.
- This is because they are not actual opcodes in the virtual machine.
- Instead, these words generate machine code in the virtual machine.
- Therefore, they are interpreted.
- When interpreted, they generate code to HERE.
- For the same reason, BEGIN, WHILE, AGAIN, and REPEAT are also interpreted.

## Reference

### Built in words
|Name       |Stack      |Description|
| ---       | ---       | --- |
| (here)    | -- a      |Address of HERE|
| (last)    | -- a      |Address of LAST|
| cell      | -- n      |Size of a CELL|
| user      | -- a      |Address of the USER area|
| user-sz   | -- n      |Size of the USER area|
|todo||fill this in|

## Assembler for the virtual machine language

Notes:
- The opcodes in cf are human-readable, so machine language code can be enterec directly.

|Opcode |Description|
| ---   | --- |
| \{    | Start FOR loop|
| \}    | End of FOR loop: NEXT|
| I     | Index for both FOR and DO loops|
| \[    | Start DO loop|
| \]    | End of FOR loop: LOOP|
| rX    | Push contents of register N on the stack|
| sX    | Pop stack to register N|
| iX    | Increment register N|
| dX    | Decrement register N|
|todo|fill this in|

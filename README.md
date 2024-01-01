# cf - a ColorForth inspired system

## What is cf?
- cf is NOT ColorForth, but it does use markers in the source to control its mode.
- cf is a byte-coded system written in C.
- cf uses the VM from c3. See this for details: https://github.com/CCurl/c3
- cf has 6 modes: DEFINE, COMMENT, INLINE, COMPILE, MACHINE, and INTERPRET.
- cf uses a either a marker code (in the whitespace) or a word to identify each mode.
- cf includes a simple text editor that will color the code to identify how it will be processed.
- cf is a work in progress.

- The marker colors as shown in the editor:
    - DEFINE: red
    - COMMENT: green
    - INLINE: orange
    - COMPILE: blue
    - UNUSED: purple
    - MACHINE: cyan
    - INTERPRET: white

- The marker codes:
    - DEFINE: ctrl-a (1)
    - COMMENT: ctrl-b (2)
    - INLINE: ctrl-c (3)
    - COMPILE: ctr-d (4)
    - UNUSED: ctrl-e (5)
    - MACHINE: ctrl-f (6)
    - INTERPRET: ctrl-g (7)

- The marker words:
    - DEFINE: '::'
    - COMMENT: '(('
    - INLINE: ':I'
    - COMPILE: '[['
    - MACHINE: ':M'
    - INTERPRET: ']]'

## Notes:
- Words like IF/THEN and BEGIN/WHILE are defined in block 0.
- They are WHITE (INTERPRETED) because they generate code.
- BEGIN, WHILE, AGAIN, and REPEAT are also interpreted.

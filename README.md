# CF: a minimal Forth

CF is a minimal Forth system that can run stand-alone or be embedded into another program.

CF has 56 base primitives.<br/>
CF is implemented in 3 files: (cf-vm.c, cf-vm.h, system.c). <br/>
The VM itself is under 200 lines of code.

In a CF program, each instruction is a CELL (32- or 64-bits). <br/>
- If <= the last primitive (system), then it is a primitive.
- Else, if the top 3 bits are set, then it is a literal.
- Else, it is the XT (code address) of a word in the dictionary.

### CF hard-codes the following IMMEDIATE state-change words:

| Word | Action |
|:--   |:-- |
|  :   | Add the next word to the dictionary, set STATE to COMPILE. |
|  ;   | Compile EXIT and change state to INTERPRET. |

**NOTE**: '(' skips words until the next ')' word.<br/>
**NOTE**: '\\' skips words until the end of the line.<br/>
**NOTE**: Setting state to 999 signals CF to exit.<br/>

## INLINE words

An INLINE word is somewhat similar to a macro in other languages.<br/>
When a word is INLINE, its definition is copied to the target, up to the first `exit`.<br/>
When not INLINE, a call is made to the word instead.

## Transient words

Words 't0' through 't9' are transient and are not added to the dictionary.<br/>
They are case sensitive: 't0' is a transient word, 'T0' is not.<br/>
They help with factoring code and and keep the dictionary uncluttered.<br/>

## CF Startup Behavior

On startup, CF does the following:
- Create 'argc' with the count of command-line arguments
- For each argument, create 'argX' with the address of the argument string
- E.G. "arg0 ztype" will print `cf`
- If arg1 exists and names a file that can be opened, load that file.
- Else, try to load file 'cf-boot.fth' in the local folder '.'.
- Else, try to load file '`BIN_DIR`cf-boot.fth' in the "bin" folder.
- On Linux, `BIN_DIR` is "/home/chris/bin/".
- On Windows, `BIN_DIR` is "D:\\bin\\".
- `BIN_DIR` is defined in cf-vm.h. Change it as appropriate for your system.

## The VM Primitives

| Primitive | Op/Word  | Stack        | Description |
|:--        |:--       |:--           |:-- |
|   0       | exit     | (--)         | PC = R-TOS. Discard R-TOS. If (PC=0) then stop. |
|   1       | lit      | (--)         | Push code[PC]. Increment PC. |
|   2       | jmp      | (--)         | PC = code[PC]. |
|   3       | jmpz     | (n--)        | If (TOS==0) then PC = code[PC] else PC = PC+1. Discard TOS. |
|   4       | jmpnz    | (n--)        | If (TOS!=0) then PC = code[PC] else PC = PC+1. Discard TOS. |
|   5       | njmpz    | (n--n)       | If (TOS==0) then PC = code[PC] else PC = PC+1. |
|   6       | njmpnz   | (n--n)       | If (TOS!=0) then PC = code[PC] else PC = PC+1. |
|   7       | dup      | (n--n n)     | Push TOS. |
|   8       | drop     | (n--)        | Discard TOS. |
|   9       | swap     | (a b--b a)   | Swap TOS and NOS. |
|  10       | over     | (a b--a b a) | Push NOS. |
|  11       | !        | (n a--)      | CELL store NOS through TOS. Discard TOS and NOS. |
|  12       | @        | (a--n)       | CELL fetch TOS through TOS. |
|  13       | c!       | (b a--)      | BYTE store NOS through TOS. Discard TOS and NOS. |
|  14       | c@       | (a--b)       | BYTE fetch TOS through TOS. |
|  15       | >r       | (n--)        | Push TOS onto the return stack. Discard TOS. |
|  16       | r@       | (--n)        | Push R-TOS. |
|  17       | r>       | (--n)        | Push R-TOS. Discard R-TOS. |
|  18       | +L       | (--)         | Allocate 3 locals (x,y,z). |
|  19       | -L       | (--)         | De-allocate last set of locals. |
|  20       | x!       | (n--)        | Set local variable X to n. |
|  21       | y!       | (n--)        | Set local variable Y to n. |
|  22       | z!       | (n--)        | Set local variable Z to n. |
|  23       | x@       | (--n)        | Push local variable X. |
|  24       | y@       | (--n)        | Push local variable Y. |
|  25       | z@       | (--n)        | Push local variable Z. |
|  26       | *        | (a b--c)     | TOS = NOS*TOS. Discard NOS. |
|  27       | +        | (a b--c)     | TOS = NOS+TOS. Discard NOS. |
|  28       | -        | (a b--c)     | TOS = NOS-TOS. Discard NOS. |
|  29       | /mod     | (a b--r q)   | TOS = NOS/TOS. NOS = NOS modulo TOS. |
|  30       | 1+       | (a--b)       | TOS = TOS+1. |
|  31       | 1-       | (a--b)       | TOS = TOS-1. |
|  32       | <        | (a b--f)     | If (NOS<TOS) then TOS = 1 else TOS = 0. Discard NOS. |
|  33       | =        | (a b--f)     | If (NOS=TOS) then TOS = 1 else TOS = 0. Discard NOS. |
|  34       | >        | (a b--f)     | If (NOS<TOS) then TOS = 1 else TOS = 0. Discard NOS. |
|  35       | 0=       | (n--f)       | If (TOS==1) then TOS = 1 else TOS = 0. |
|  36       | +!       | (n a--)      | Add NOS to the cell at TOS. Discard TOS and NOS. |
|  37       | for      | (C--)        | Start a FOR loop starting at 0. Upper limit is C. |
|  38       | i        | (--I)        | Push current loop index. |
|  39       | next     | (--)         | Increment I. If I < C then jump to loop start. |
|  40       | and      | (a b--c)     | TOS = NOS and TOS. Discard NOS. |
|  41       | or       | (a b--c)     | TOS = NOS or  TOS. Discard NOS. |
|  42       | xor      | (a b--c)     | TOS = NOS xor TOS. Discard NOS. |
|  43       | ztype    | (a--)        | Output null-terminated string TOS. Discard TOS. |
|  44       | find     | (--a)        | Push the dictionary address of the next word. |
|  45       | key      | (--n)        | Push the next keypress. Wait if necessary. |
|  46       | key?     | (--f)        | Push 1 if a keypress is available, else 0. |
|  47       | emit     | (c--)        | Output char TOS. Discard TOS. |
|  48       | fopen    | (nm md--fh)  | Open file NOS using mode TOS (h=0 if error). |
|  49       | fclose   | (fh--)       | Close file TOS. Discard TOS. |
|  50       | fread    | (a sz fh--n) | Read NOS chars from file TOS to a. |
|  51       | fwrite   | (a sz fh--n) | Write NOS chars from file TOS from a. |
|  52       | ms       | (n--)        | Wait/sleep for TOS milliseconds |
|  53       | timer    | (--n)        | Push the current system time. |
|  54       | add-word | (--)         | Add the next word to the dictionary. |
|  55       | outer    | (a--)        | Run the outer interpreter on TOS. Discard TOS. |
|  56       | system   | (a--)        | Execute system(TOS). Discard TOS. |

## Other built-in words

| Word      | Stack | Description |
|:--        |:--    |:-- |
| version   | (--n) | Current version number. |
| output-fp | (--a) | Address of the output file handle. 0 means STDOUT. |
| (h)       | (--a) | Address of HERE. |
| (l)       | (--a) | Address of LAST. |
| (lsp)     | (--a) | Address of the loop stack pointer. |
| lstk      | (--a) | Address of the loop stack. |
| (rsp)     | (--a) | Address of the return stack pointer. |
| rstk      | (--a) | Address of the return stack. |
| (sp)      | (--a) | Address of the data stack pointer. |
| stk       | (--a) | Address of the data stack. |
| state     | (--a) | Address of STATE. |
| base      | (--a) | Address of BASE. |
| mem       | (--a) | Address of the beginning of the memory area. |
| mem-sz    | (--n) | The number of BYTEs in the memory area. |
| >in       | (--a) | Address of the text input buffer pointer. |
| cell      | (--n) | The size of a CELL in bytes (4 or 8). |

##   Embedding CF in your C or C++ project

See system.c. It embeds the CF VM into a C program.

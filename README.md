# CF: a minimal DWORD-Code Forth

CF is an extremely minimal Forth system that can run stand-alone or be embedded into another program.

CF has 32 base primitives, 14 system primitives.<br/>
CF is implemented in 3 files: (cf-vm.c, cf-vm.h, system.c). <br/>
The VM itself is under 200 lines of code.

On Windows, a 32-bit Release build compiles to a 17k executable. <br/>
On a Linux box, it is about 21k.

**CF** stands for "DWord-Code". This is inspired by Tachyon. <br/>
In a CF program, each instruction is a DWORD (32-bits). <br/>
- If <= the last primitive (45), then it is a primitive.
- Else, if the top 3 bits are set, then it is a literal ANDed with $3FFFFFFF.
- Else, it is the XT (code address) of a word in the dictionary.

### CF hard-codes the following IMMEDIATE state-change words:

| Word | Action |
|:--   |:-- |
|  :   | Add the next word to the dictionary, set STATE to COMPILE. |
|  ;   | Compile EXIT and change state to INTERPRET. |

**NOTE**: '(' skip words until the next ')' word.<br/>
**NOTE**: '\\' skip words until the end of the line.<br/>
**NOTE**: State '999' signals CF to exit.<br/>

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
- Else, try to load file 'boot.fth'

## The VM Primitives

| Primitive | Op/Word  | Stack        | Description |
|:--        |:--       |:--           |:-- |
|           |          |              | --- **CF primitives** --- |
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
|  18       | *        | (a b--c)     | TOS = NOS*TOS. Discard NOS. |
|  19       | +        | (a b--c)     | TOS = NOS+TOS. Discard NOS. |
|  20       | -        | (a b--c)     | TOS = NOS-TOS. Discard NOS. |
|  21       | /mod     | (a b--r q)   | TOS = NOS/TOS. NOS = NOS modulo TOS. |
|  22       | <        | (a b--f)     | If (NOS<TOS) then TOS = 1 else TOS = 0. Discard NOS. |
|  23       | =        | (a b--f)     | If (NOS=TOS) then TOS = 1 else TOS = 0. Discard NOS. |
|  24       | >        | (a b--f)     | If (NOS<TOS) then TOS = 1 else TOS = 0. Discard NOS. |
|  25       | +!       | (n a--)      | Add NOS to the cell at TOS. Discard TOS and NOS. |
|  26       | for      | (C--)        | Start a FOR loop starting at 0. Upper limit is C. |
|  27       | i        | (--I)        | Push current loop index. |
|  28       | next     | (--)         | Increment I. If I < C then jump to loop start. |
|  29       | and      | (a b--c)     | TOS = NOS and TOS. Discard NOS. |
|  30       | or       | (a b--c)     | TOS = NOS or  TOS. Discard NOS. |
|  31       | xor      | (a b--c)     | TOS = NOS xor TOS. Discard NOS. |
|           |          |              | --- **System primitives** --- |
|  32       | ztype    | (a--)        | Output null-terminated string TOS. Discard TOS. |
|  33       | find     | (--a)        | Push the dictionary address of the next word. |
|  34       | key      | (--n)        | Push the next keypress. Wait if necessary. |
|  35       | key?     | (--f)        | Push 1 if a keypress is available, else 0. |
|  36       | emit     | (c--)        | Output char TOS. Discard TOS. |
|  37       | fopen    | (nm md--fh)  | Open file NOS using mode TOS (h=0 if error). |
|  38       | fclose   | (fh--)       | Close file TOS. Discard TOS. |
|  39       | fread    | (a sz fh--n) | Read NOS chars from file TOS to a. |
|  40       | fwrite   | (a sz fh--n) | Write NOS chars from file TOS from a. |
|  41       | ms       | (n--)        | Wait/sleep for TOS milliseconds |
|  42       | timer    | (--n)        | Push the current system time. |
|  43       | add-word | (--)         | Add the next word to the dictionary. |
|  44       | outer    | (a--)        | Run the outer interpreter on TOS. Discard TOS. |
|  45       | system   | (a--)        | Execute system(TOS). Discard TOS. |

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

##   Embedding CF in your C or C++ project

See system.c. It embeds the CF VM into a C program.

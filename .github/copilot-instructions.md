# CF - ColorForth-Inspired Forth VM

## Architecture Overview

CF is a minimal Forth virtual machine written in C that takes heavy inspiration from ColorForth and Tachyon. The system consists of three core C files:
- `cf.c` - VM implementation, inner/outer interpreters, primitives
- `cf.h` - API, configuration (MEM_SZ, STK_SZ, CELL_SZ), type definitions
- `system.c` - Platform-specific I/O, embeds CF into executable

**Memory model**: Single memory chunk (`mem[]`) holds both code and data. Code area starts at `mem[0]`, dictionary grows down from top, variable space (`vhere`) defined in boot file. Dictionary entries are `DE_T` structs at the high end of memory.

**OPCODE encoding** (critical to understanding execution):
- If `opcode <= BYE`: it's a primitive enum value
- If top 3 bits set (`NUM_BITS`): it's an inline literal (mask with `NUM_MASK`)
- Otherwise: it's an XT (execution token) address to call

## State-Based Parser

CF uses whitespace control characters (bytes 1-4) to manage state:
- `\x01` or `:` → DEFINE (then auto-switches to COMPILE)
- `\x02` or `]` → COMPILE
- `\x03` or `[` → INTERPRET
- `\x04` → COMMENT
- `;` compiles EXIT and switches to INTERPRET

State is stored in the global `state` variable. The `cfOuter()` function in `cf.c` is the outer interpreter that processes source code according to current state.

## Key Data Structures

**Dictionary Entry (`DE_T`)**: Fixed-size struct with `xt` (execution token), `flags` (_IMMED, _INLINE), `len`, and `name[NAME_MAX+1]`. Stored at high memory addresses, grows downward.

**Stacks**: Six stacks total - data (`dstk`), return (`rstk`), loop (`lstk`), plus ColorForth-inspired A/B/T register stacks (`astk`, `bstk`, `tstk`). Each has a stack pointer (`dsp`, `rsp`, etc.).

**A/B/T Register Stack Operations** (critical for efficient CF code):
- `>a`, `>b`, `>t` - Push value from data stack to register stack
- `a@`, `b@`, `t@` - Fetch current register value to data stack (non-destructive)
- `a!`, `b!`, `t!` - Store TOS into current register (non-destructive to register stack)
- `a@+`, `b@+`, `t@+` - Fetch register value, then increment register
- `a@-`, `b@-`, `t@-` - Fetch register value, then decrement register
- `a>`, `b>`, `t>` - Pop register stack to data stack
- Helper words in `cf-boot.fth`: `a+` (`a@+ drop`), `b+` (`b@+ drop`), `b-` (`b@- drop`) - increment/decrement without stack effects
- Compound operations: `c@a` (`a@ c@`), `c!a` (`a@ c!`), `c!a+` (`a@+ c!`), `c!b-` (`b@- c!`)

**Register Stack Naming Convention** (critical for readability):
- **Prefix `@`** = fetch the value (dereference)
- **Infix register** = which register (a, b, t)
- **Suffix modifiers** = then do this (nothing, `+`, `-`, `+cell`, etc.)
- Examples: `@a` (fetch at A), `@a+` (fetch at A, increment A), `@a+cell` (fetch at A, increment A by cell), `a@+cell` (fetch A, increment A by cell)
- `a@+` reads as: "fetch A, then increment A"
- `c@a+` reads as: "character-fetch through A, then increment A"
- `a@+cell` reads as: "fetch A, then increment A by cell"
- `@a+cell` reads as: "fetch through A, then increment A by cell"

**Register Stack Patterns**:
- **Pointer manipulation**: Store addresses in A/B, use `a@+`/`b@-` for auto-increment/decrement during loops
- **Stack effect awareness**: `c!a+` does `a@+ c!` - pushes A to stack, increments A, then stores TOS at that address
- **Natural swap**: `c@a c@b c!a+ c!b-` swaps bytes at A and B without explicit `swap` - stack order does it automatically
- **Comparison direction**: When A increments upward and B decrements downward, use `a@ b@ >=` to detect when pointers meet/cross
- **Prefer register operations over data stack juggling** - reduces `dup`, `over`, `swap` overhead

**CRITICAL: Register Stack Balance Rule**:
**Every `>a`/`>b`/`>t` MUST have a corresponding `a>`/`b>`/`t>` or `adrop`/`bdrop`/`tdrop` before the word exits.** This applies to ALL exit paths (including early `exit` in conditionals). Register stack imbalance is a bug, just like data stack imbalance. When generating CF code, maintain strict register stack discipline - what goes on must come off within each word definition.

**Idiomatic Register Usage** (beyond balance):
Balance is necessary but not sufficient. Write *idiomatic* CF by leveraging register semantics:
- **Let registers do their job**: Operations like `a@+`, `b@-`, `t@-` are designed to modify state. Use them naturally rather than fighting against them with calculations.
- **Non-destructive vs. destructive operations matter**: `a@` leaves A unchanged; `a@+` increments A. Choose based on intent, not just stack effects.
- **Natural loops over computed indices**: When you need decreasing values in a loop, use `1- >t for ... t@- ... next` to let T naturally decrement, rather than trying to keep T constant and computing differences with loop indices.
- **Example - bubble sort**: Instead of `t@ i -` (fetch constant T and compute with index), use `t@-` in each iteration to naturally decrement T. This is cleaner and more aligned with CF's design.

**Temporary words**: `t0` through `t9` are special dictionary entries stored in `tmpWords[]` array, not in main dictionary. Used for transient definitions.

## Stack Effect Comments (MANDATORY)

**Every word definition MUST include a stack effect comment** immediately after the word name. This is critical for:
- Understanding inputs/outputs at a glance without reading implementation
- Debugging stack imbalances quickly
- Confidently composing words together
- Serving as inline documentation
- Future maintenance and code comprehension

Format: `: word-name ( input--output ) implementation ;`

Examples:
- `: dup ( n--n n ) ... ;` - duplicates top of stack
- `: drop ( n-- ) ... ;` - removes top of stack
- `: swap ( a b--b a ) ... ;` - exchanges top two items
- `: negate ( n--n' ) ... ;` - returns modified version of input
- `: bl ( --n ) 32 ;` - pushes constant (no input)
- `: cr ( -- ) ... ;` - no stack effect

**Stack effect notation conventions**:
- `a`, `b` - addresses (memory locations)
- `n` - number (integer value)
- `n'` - modified/transformed number (result)
- `f` - flag (boolean: 0=false, non-zero=true)
- `c`, `ch` - character (byte value)
- `s`, `str` - string address
- `d`, `dst` - destination address
- `src` - source address
- `e` - end address
- `w` - width
- `fh` - file handle
- `xt` - execution token (word address)
- `de` - dictionary entry address
- `pos` - position
- `blk` - block number
- `r` - row
- `k` - key code
- `tbl` - table address

**When creating new words, ALWAYS include stack effect comments.** The only rare exception is when the name makes it completely obvious, but even then it's better to include them.

## Adding Primitives

Primitives use the `PRIMS(X)` macro pattern in `cf.c`. The X-macro is expanded 3 times with different definitions (X1, X2, X3) to generate:
1. Enum values for opcodes
2. Switch cases in `cfInner()`
3. Dictionary initialization in `cfInit()`

Example:
```c
X(SCOPY, "s-cpy", t=pop(); strcpy((char*)TOS, (char*)t); )
```
This creates opcode `SCOPY`, word name `"s-cpy"`, and inline execution code.

## Build System

**Makefile** supports `ARCH=32` or `ARCH=64` (default). Simple build:
```bash
make              # 64-bit
ARCH=32 make      # 32-bit
```

Manual compilation uses `$CC -m32/-m64 -O3 -o cf cf.c system.c`.

Visual Studio solution (`cf.sln`) provided for Windows.

## Bootstrap Process

On startup, CF:
1. Calls `cfInit()` to initialize memory, stacks, create primitive dictionary entries
2. Attempts to load boot file (default: `BOOT_FN1` = "cf-boot.fth", fallback: `BOOT_FN2`)
3. Boot file defines Forth-layer words (`,`, `const`, `var`, flow control, etc.)
4. Boot file loads block system from `disk.cf` if present
5. Enters REPL loop in `repl()` function

The `HERE` pointer starts at `BYE+1` (first available code slot after primitives).

## Block System

Not native to CF VM - implemented in `cf-boot.fth`. Configuration:
- Block size: 2048 bytes (`blk-sz`)
- Storage: `disk.cf` file, loaded into `blks` buffer at high memory
- Current block tracked in `t0` variable
- `load` word executes block N by calling `outer` on block data
- Editor in boot file provides vi-like interface (23 rows × 89 cols)

## Development Workflow

**Testing changes**: Modify `cf.c`/`system.c`, run `make`, test with `./cf`. Boot file loads automatically.

**Modifying primitives**: Edit `PRIMS(X)` macro, rebuild. VM recompilation is fast (~1 second).

**Extending Forth layer**: Edit `cf-boot.fth` or blocks (`block-NNN.fth` files). The `rb` (reboot) word in boot file reloads from disk.

**Number literals**: Support `#123` (decimal), `$FF` (hex), `%1010` (binary), `'X'` (char). Base stored in global `base` variable.

## Platform Differences

`system.c` handles platform I/O:
- **Windows**: Uses `_kbhit()`, `_getch()`, `Sleep()`
- **Linux/BSD**: Uses termios for raw mode, `select()` for `qKey()`, `usleep()` for timing

Define `IS_WINDOWS` for Windows builds (auto-detected by MSVC).

## Important Globals

- `here`: Current code compilation address (offset into `code[]`)
- `vhere`: Current variable area address (grows upward from defined start)
- `last`: Top of dictionary (grows downward)
- `toIn`: Input stream pointer (used by `nextWord()`)
- `wd[128]`: Current word buffer

All exposed to Forth via literals created in `cfInit()`.

## Forth Bootstrap Layer (cf-boot.fth)

The `cf-boot.fth` file builds a complete Forth system on top of the C primitives. Understanding its structure is critical for extending the system.

**Memory initialization**: Sets `vhere` to 65536 cells past `memory` base, establishing the variable area boundary. Code area and dictionary are already initialized by `cfInit()`.

**Core defining words**:
- `,` - Compiles an opcode into the code area at `here`, then increments `here`
- `const` - Creates a constant by compiling: `addword`, `lit,`, the value, then `exit`
- `var` - Allocates variable space: calls `vhere`, makes it a constant, then calls `allot`
- `immediate` - Sets flag byte 1 at `last + cell` to mark word as immediate
- `inline` - Sets flag byte 2 to mark word for inline compilation

**Flow control** (all immediate):
- `begin`/`until`/`while`/`again` - Loop constructs using `jmpz`, `jmpnz`, `jmp` opcodes
- Variants with `-` prefix (`-while`, `-until`, `-if`) use non-destructive jumps (`njmpz`, `njmpnz`)
- `if`/`if0`/`-if`/`then` - Conditional branches. `then` backpatches the jump address using `code!`

**String handling**:
- `z"` and `."` - Quote parser using temporary word `t4`
- `t4` uses A/B stacks: `>in` pointer in A, destination buffer in B
- Scans input until `"`, null-terminates string, updates `>in`, compiles literal if in COMPILE mode

**Block system implementation**:
- Fixed 2048-byte blocks (`blk-sz`), max 128 blocks
- Block buffer at `memory + mem-sz - 2000000` (high memory location)
- Current block number stored in `t0` temporary variable
- `disk-read`/`disk-write` - Load/save entire `disk.cf` file (256KB)
- `load` - Executes block N by null-terminating and calling `outer` on block data
- Automatic block load on boot via `disk-read` at end of bootstrap

**Editor (`ed`)**:
- Vi-like modal editor (23 rows × 89 columns)
- Three modes: normal (0), replace (1), insert (2)
- Switch-based command dispatch using case tables (`ed-cases`, `ed-ctrl-cases`, `ed-del-cases`)
- Case table format: each entry is `char, xt` pair, terminated by `0, 0`
- Color-coded syntax by state: purple (default), red (define), green (compile), yellow (interpret), white (comment)
- F1-F4 keys insert state control bytes (1-4) directly into block
- Commands: `:` enters Forth command mode, `w` writes block, `q` quits, `=/-` navigate blocks

**Utility patterns**:
- Stack-based string operations use A/B/T stacks heavily (e.g., `s-cat`, `s-rtrim`)
- Number formatting: `<#`, `#`, `#s`, `#>` pattern builds strings backward in buffer
- `marker`/`forget` - Save/restore dictionary state using `t0`/`t1`/`t2` temporaries
- `rb` (reboot) - Reloads `cf-boot.fth` by restoring saved `last`/`here` values and re-executing

**VT100 terminal control**:
- ANSI escape sequences via `csi` helper (emits `ESC [`)
- Cursor positioning: `->rc`, `->cr` (row/col to screen coordinates)
- Color support: `fg`/`bg` using 256-color palette, named color words
- `vkey` - Cross-platform key handler (VT sequences on Linux/BSD, scan codes on Windows)

**Development workflow helpers**:
- `words` - Dictionary browser with column formatting
- `.s` - Non-destructive stack display
- `:noname` - Anonymous word definition (returns xt)
- `execute` - Execute xt via return stack (`>r`)

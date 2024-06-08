# cf - a ColorForth inspired system

# NOTE: I know this is out of date; I have to update it!

<img src="/images/editing.jpg" width="250" height="250" />

## What is cf?
- cf is inspired by ColorForth, but it is NOT ColorForth.
- Like ColorForth, cf uses markers in the source to control its mode.
- cf uses the VM from c4. See this for details: https://github.com/CCurl/c4
- cf has 5 modes: DEFINE, COMMENT, MACRO, COMPILE, INTERPRET.
- cf supports using either a marker byte in the whitespace or a word to identify each mode.
- cf includes a simple block editor that colors the file to identify how it will be processed.
- cf is a work in progress. **Please collaborate with me on it!**

## Notes:
- Words like IF/THEN are defined in block 1.
- They are BLUE (MACRO) because they generate code.
- BEGIN, WHILE, UNTIL, and AGAIN are also interpreted.

## The markers:

|Mode       |Color   | Code  |Editor  | Word(s) |
| :--       | :--    | :--   | :--    | :--     |
| DEFINE    | red    | 1     | ctrl-a | ":"     |
| COMPILE   | green  | 2     | ctrl-b | "]" or ")"  |
| INTERPRET | orange | 3     | ctrl-c | "[" or "))" |
| MACRO     | blue   | 4     | ctrl-d | "[" or "))" |
| COMMENT   | white  | 7     | ctrl-g | "(" or "((" |

## Blocks
- In cf, a block is a file with the name "block-NNN.cf". It has a maximum of BLOCK_SZ bytes.
- BLOCK_SZ, in cf.h, is defaulted to 2500. It is easy to change as needed.

## The Editor

The editor supports 25 lines of 100 characters each.

The editor has 3 modes: NORMAL, INSERT and REPLACE.

## NORMAL MODE

### Moving around

|  Key     | Action |
| :--      | :--    |
| h/j/k/l  | Move cursor left/down/up/right (like VI) |
| [bs]     | Move cursor left 1 character. (back-space) |
| [ctrl-h] | Move cursor left 1 character. |
| [ctrl-j] | Move cursor down 1 line. |
| [ctrl-k] | Move cursor up 1 line. |
| [ctrl-l] | Move cursor right 1 character. |
| [space]  | Move cursor right 1 character |
| [tab]    | Move cursor right 8 characters. |
| q        | Move cursor right 8 characters. |
| Q        | Move cursor left 8 characters. |
| _        | Go to BOL (the Beginning Of the Line) |
| $        | Go to EOL (the End Of the Line) |
| g        | Go to the beginning of the block |
| G        | Go to the beginning of the last line of the block |
| :        | Execute a command. |

### ColorForth markers
|  Key     | Action |
| :--      | :--    |
| [ctrl-a] | Insert a space if necessary. Place DEFINE marker. |
| [ctrl-b] | Insert a space if necessary. Place COMPILE marker. |
| [ctrl-c] | Insert a space if necessary. Place INTERPRET marker. |
| [ctrl-d] | Insert a space if necessary. Place MACRO marker. |
| [ctrl-e] | (Unused) |
| [ctrl-f] | (Unused) |
| [ctrl-g] | Insert a space if necessary. Place COMMENT marker. |

### Modifying text
|  Key     | Action |
| :--      | :--    |
| c        | Delete the current character and change MODE to INSERT |
| C        | Delete to EOL and change MODE to INSERT |
| D        | Delete the current line into the YANK buffer |
| i        | Change MODE to INSERT |
| o        | Open a line below the current line and change MODE to REPLACE |
| O        | Open a line above the current line and change MODE to REPLACE |
| p        | Paste the yanked line after the current line |
| P        | Paste the yanked line before the current line |
| r        | Replace the character under the cursor with the next key pressed |
| R        | Change MODE to REPLACE |
| x        | Delete the current character to the end of the line. |
| X        | Delete the current character to the end of the block. |
| [ctrl-x] | Move cursor left 1 character. Delete character to the end of the line. |
| Y        | YANK/copy the current line into the YANK buffer. |

### COMMANDs
|  Key     | Action |
| :--      | :--    |
| +        | Save current block if changed and edit block+1 |
| -        | Save current block if changed and edit block-1 |
| :rl      | Reload the block from disk |
| :w       | Write the block to disk |
| :wq      | Write and then Quit |
| :q       | Quit the editor if no changes made. |
| :q!      | Quit the editor even if there are changes. |
| [esc]    | Cancel current operation and change MODE to NORMAL |

### INSERT Mode

|  Key     | Action |
| :--      | :--    |
| [bs]     | Move cursor left 1 character. |
| [ctrl-h] | Move cursor left 1 character. |
| [ctrl-j] | Move cursor down 1 line. |
| [ctrl-k] | Move cursor up 1 line. |
| [ctrl-l] | Move cursor right 1 character. |
| [ctrl-x] | Move cursor left 1 character. Delete character. |
| [esc]    | Cancel and change MODE to COMMAND |
| [other]  | Insert the char and move the cursor right. |


### REPLACE Mode

|  Key     | Action |
| :--      | :--    |
| [esc]    | Cancel and change MODE to COMMAND |
| [bs]     | Move cursor left 1 character. |
| [ctrl-h] | Move cursor left 1 character. |
| [ctrl-j] | Move cursor down 1 line. |
| [ctrl-k] | Move cursor up 1 line. |
| [ctrl-l] | Move cursor right 1 character. |
| [ctrl-x] | Move cursor left 1 character. Delete character. |
| [cr]     | Move to the beginning of the next line |
| [other]  | Insert the char and move the cursor right. |

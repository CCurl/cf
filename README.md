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
- In cf, a block is a file with the name "block-NNN.cf". For the built-in editor, it has a maximum of BLOCK_SZ bytes. For other editors, there is no limit.
- BLOCK_SZ, in cf.h, is defaulted to 2500. It is easy to change as needed.

## The Editor

The editor supports 25 lines of 100 characters each.

The editor has 3 modes: NORMAL, INSERT and REPLACE.

## Commmon actions
|  Key     | Action |
| :--      | :--    |
| [ctrl-a] | Insert a space if necessary. Place DEFINE marker. |
| [ctrl-b] | Insert a space if necessary. Place COMPILE marker. |
| [ctrl-c] | Insert a space if necessary. Place INTERPRET marker. |
| [ctrl-d] | Insert a space if necessary. Place MACRO marker. |
| [ctrl-e] | (Unused) |
| [ctrl-f] | (Unused) |
| [ctrl-g] | Insert a space if necessary. Place COMMENT marker. |
| [ctrl-h] | Move cursor left 1 character. |
| [ctrl-i] | Move cursor right 8 characters. |
| [ctrl-j] | Move cursor down 1 line. |
| [ctrl-k] | Move cursor up 1 line. |
| [ctrl-l] | Move cursor right 1 character. |
| [ctrl-m] | Move cursor down 1 and to the beginning of the line. |
| [ctrl-n] | (Unused) |
| [ctrl-o] | (Unused) |
| [ctrl-p] | (Unused) |
| [ctrl-q] | Move cursor left 8 characters. |
| [ctrl-r] | (Unused) |
| [ctrl-s] | (Unused) |
| [ctrl-t] | (Unused) |
| [ctrl-u] | Move cursor down 5 lines |
| [ctrl-v] | Move cursor up 5 lines |
| [ctrl-w] | (Unused) |
| [ctrl-x] | Move left 1 and delete (line-only). |
| [ctrl-y] | Delete current char (line-only). |
| [enter]  | Move cursor down 1 and to the beginning of the line. |
| [tab]    | Move cursor right 8 characters. |
| [bs]     | Windows: [ctrl-h], Linux: [ctrl-x] |

## NORMAL MODE

|  Key     | Action |
| :--      | :--    |
| [space]  | Move right 1 character. |
| b        | Insert a space (line-only). |
| B        | Insert a space. |
| C        | Delete to EOL and change MODE to INSERT. |
| d[x]     | [x] identifies what to delete. |
| D        | Delete the current line into the YANK buffer. |
| g        | Move to the beginning of the block. |
| G        | Move to the beginning of the last line of the block. |
| h/j/k/l  | Move cursor left/down/up/right (like VI). |
| i        | Change MODE to INSERT. |
| I        | Move to the beginning of the line and change MODE to INSERT. |
| o        | Open a line below the current line and change MODE to REPLACE. |
| O        | Open a line above the current line and change MODE to REPLACE. |
| p        | Paste the yanked line after the current line. |
| P        | Paste the yanked line before the current line. |
| q        | Move right 8 characters. |
| Q        | Move left 8 characters. |
| r        | Replace the character under the cursor with the next key pressed. |
| R        | Change MODE to REPLACE. |
| x        | Delete the current character (line-only). |
| X        | Delete the current character. |
| Y        | YANK/copy the current line into the YANK buffer. |
| _        | Move to the beginning of the line. |
| $        | Move to the end Of the line. |
| :        | Execute a command. |

### Deleting
The combination d[x] identifies what to delete.

|  Combo   | Action |
| :--      | :--    |
| d.       | Delete the current character to the end of the line (same as x). |
| d$       | Delete to the end of the line. |
| d_       | Delete to the beginning of the line. |
| dd       | Delete the current line into the YANK buffer (same as D). |

### COMMANDs
| Command  | Action |
| :--      | :--    |
| +        | Save current block if changed and edit block+1 |
| -        | Save current block if changed and edit block-1 |
| :rl      | Reload the block from disk |
| :w       | Write the block to disk |
| :wq      | Write and then Quit |
| :q       | Quit the editor if no changes made. |
| :q!      | Quit the editor even if there are changes. |
| [esc][x] | Cancel and change MODE to NORMAL. |

### INSERT Mode

|  Key     | Action |
| :--      | :--    |
| [ctrl-z] | Cancel and change MODE to NORMAL. |
| [esc][x] | Cancel and change MODE to NORMAL. |
| [other]  | Insert the char and move the cursor right. |

### REPLACE Mode

|  Key     | Action |
| :--      | :--    |
| [ctrl-z] | Cancel and change MODE to NORMAL. |
| [esc][x] | Cancel and change MODE to NORMAL. |
| [other]  | Insert the char and move the cursor right. |

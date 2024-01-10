# cf - a ColorForth inspired system

<img src="/images/editing.jpg" width="250" height="250" />

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

The editor supports 20 lines of 100 characters each.

The editor has 3 modes: COMMAND, INSERT and REPLACE

## COMMAND MODE

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
| _        | Go to BOL (the Beginning Of the Line) |
| $        | Go to EOL (the End Of the Line) |
| g        | Go to the beginning of the block |
| G        | Go to the beginning of the last line of the block |

### ColorForth markers
|  Key     | Action |
| :--      | :--    |
| [ctrl-a] | Insert a space if necessary. Place DEFINE marker. |
| [ctrl-b] | Insert a space if necessary. Place COMMENT marker. |
| [ctrl-c] | Insert a space if necessary. Place INLINE marker. |
| [ctrl-d] | Insert a space if necessary. Place UNUSED marker. |
| [ctrl-e] | Insert a space if necessary. Place MACHINE marker. |
| [ctrl-f] | Insert a space if necessary. Place INTERPRET marker. |

### Modifying text
|  Key     | Action |
| :--      | :--    |
| a        | Move right one char and change MODE to INSERT |
| A        | Go to EOL and change MODE to INSERT |
| c        | Delete the current character and change MODE to INSERT |
| C        | Delete to EOL and change MODE to INSERT |
| D        | Delete the current line |
| i        | Change MODE to INSERT |
| I        | Go to BOL and change MODE to INSERT |
| J        | Join the current and next line |
| o        | Open a line after the current line and change MODE to INSERT |
| O        | Open a line before the current line and change MODE to INSERT |
| p        | Paste the yanked line after the current line |
| P        | Paste the yanked line before the current line |
| r        | Replace the character under the cursor with the next key pressed |
| R        | Change MODE to REPLACE |
| x        | Delete the current character |
| X        | Move cursor left 1 character. Delete character. |
| [ctrl-x] | Move cursor left 1 character. Delete character. |
| Y        | Yank/copy the current line |

### Other Commands
|  Key     | Action |
| :--      | :--    |
| +        | Save current block if changed and edit block+1 |
| -        | Save current block if changed and edit block-1 |
| L        | Reload the block from disk |
| W        | Write the block to disk |
| Q        | Quit the editor (does not save automatically) |
| [esc]    | Cancel and change MODE to COMMAND |

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
| [bs]     | Move left and delete char. |
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
| [cr]     | Move the be beginning of the next line |
| [other]  | Insert the char and move the cursor right. |

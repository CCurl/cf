// editor.cpp - A simple editor

#include "cf.h"

#define COMMAND       1
#define INSERT        2
#define REPLACE       3

enum { curLEFT = 200, curRIGHT, curUP, curDOWN, curHOME, curPGUP, curPGDN, curEND, delCH };

#define LLEN       100
#define NUM_LINES   20
#define BLOCK_SZ    (NUM_LINES)*(LLEN)
#define MAX_CUR     (BLOCK_SZ-1)
#define SETC(c)     edLines[line][off]=c
#define BL          32

char theBlock[BLOCK_SZ];
int line, off, blkNum, edMode;
int isDirty = 0;
const char* msg = NULL;
char edLines[NUM_LINES][LLEN];

void GotoXY(int x, int y) { printStringF("\x1B[%d;%dH", y, x); }
void CLS() { printString("\x1B[2J"); GotoXY(1, 1); }
void CursorOn() { printString("\x1B[?25h"); }
void CursorOff() { printString("\x1B[?25l"); }

void NormLO() {
    if (line < 0) { line = 0; }
    if (NUM_LINES <= line) { line = (NUM_LINES-1); }
    if (off < 0) { off = 0; }
    if (LLEN <= off) { off = (LLEN-1); }
}

int edColor(int ch, int prev) {
    if (prev != BL) { return 0; }
    if (ch == DEFINE) { return DEFINE; }
    if (ch == COMMENT) { return COMMAND; }
    if (ch == COMPILE) { return COMPILE; }
    if (ch == IMMED) { return IMMED; }
    return 0;
}

char edChar(int l, int o) {
    char ch = edLines[l][o];
    char prev = (0 < o) ? edLines[l][o-1] : BL;
    int color = edColor(ch, prev);
    if (color) { cfColor(color); ch = BL; }
    return (31<ch) ? ch : BL;
}

void showLine(int l) {
    // CursorOff();
    GotoXY(1, l + 1);
    for (int o = 0; o < LLEN; o++) {
        printChar(edChar(l, o));
    }
    // CursorOn();
}

void showCursor() {
    char c = edChar(line, off);
    GotoXY(off + 1, line + 1);
    Color(0,47);
    printChar(c ? c : 'X');
}

void mv(int l, int o) {
    showLine(line);
    line += l;
    off += o;
    NormLO();
    showLine(line);
    showCursor();
}

void edSetCh(char c) {
    SETC(c);
    mv(0, 1);
    isDirty = 1;
}

int edKey() {
    CursorOn();
    int c = key();
    CursorOff();
    // in PuTTY, cursor keys are <esc>, '[', [A..D]
    // other keys are <esc>, '[', [1..6] , '~'
    if (c == 27) {
        c = key();
        if (c == '[') {
            c = key();
            if (c == 'A') { return curUP; } // up
            if (c == 'B') { return curDOWN; } // down
            if (c == 'C') { return curRIGHT; } // right
            if (c == 'D') { return curLEFT; } // left
            if (c == '1') { if (key() == '~') { return curHOME; } } // home
            if (c == '4') { if (key() == '~') { return curEND; } } // end
            if (c == '5') { if (key() == '~') { return curPGUP; } } // top (pgup)
            if (c == '6') { if (key() == '~') { return curPGDN; } } // last (pgdn)
            if (c == '3') { if (key() == '~') { return delCH; } } // del
        }
        return c;
    }
    else {
        // in Windows, cursor keys are 224, [HPMK]
        // other keys are 224, [GOIQS]
        if (c == 224) {
            c = key();
            if (c == 'H') { return curUP; } // up
            if (c == 'P') { return curDOWN; } // down
            if (c == 'M') { return curRIGHT; } // right
            if (c == 'K') { return curLEFT; } // left
            if (c == 'G') { return curHOME; } // home
            if (c == 'O') { return curEND; } // end
            if (c == 'I') { return curPGUP; } // pgup
            if (c == 'Q') { return curPGDN; } // pgdn
            if (c == 'S') { return delCH; } // del
            return c;
        }
    }
    return c;
}

void clearBlock() {
    for (int i = 0; i < BLOCK_SZ; i++) {
        theBlock[i] = 0;
        // if ((i % 50) == 0) { theBlock[i] = 10; }
    }
}

int toBlock() {
    int c = 0;
    clearBlock();
    for (int y = 0; y < NUM_LINES; y++) {
        for (int x = 0; x < LLEN; x++) {
            theBlock[c++] = edLines[y][x];
        }
        int x=LLEN-1;
        while ((theBlock[c-1] < 33) && (0 <= x)) { --c; --x; }
        theBlock[c++]=10;
    }
    return c;
}

void clearLine(int y) {
    for (int x = 0; x < LLEN; x++) { edLines[y][x] = 0; }
    //edLines[y][LLEN-1] = 0;
}

void toLines() {
    int x = 0, y = 0, ch;
    for (int i = 0; i < NUM_LINES; i++) { clearLine(i); }
    for (int i = 0; i < BLOCK_SZ; i++) {
        ch = theBlock[i];
        if (ch == 0) { return; }
        if (ch ==10) {
            if (NUM_LINES <= (++y)) { return; }
            x=0; continue;
        }
        if ((x < LLEN) && (31 < ch)) { edLines[y][x++] = (char)ch; }
    }
}

void edRdBlk() {
    char buf[24];
    clearBlock();
    sprintf(buf, "block-%03d.cf", blkNum);
    msg = "-noFile-";
    FILE* fp = fopen(buf, "rb");
    if (fp) {
        fread(theBlock, 1, BLOCK_SZ, fp);
        msg = "-loaded-";
        fclose(fp);
    }
    toLines();
    isDirty = 0;
}

void edSvBlk() {
    int sz = toBlock();
    char buf[24];
    sprintf(buf, "block-%03d.cf", blkNum);
    msg = "-err-";
    FILE* fp = fopen(buf, "wb");
    if (fp) {
        fwrite(theBlock, 1, sz, fp);
        msg = "-saved-";
        fclose(fp);
    }
    // msg = (pop()) ? "-saved-" : "-errWrite-";
    isDirty = 0;
}

void showFooter() {
    GotoXY(1, NUM_LINES+1);
    printString("- Block Editor v0.1 - ");
    printStringF("Block# %03d %c", blkNum, isDirty ? '*' : BL);
    printStringF(" %s -\r\n", msg ? msg : "");
    printString("\r\n  (q)home (w)up (e)end (a)left (s)down (d)right (t)op (l)ast");
    printString("\r\n  (x)del (r)Replace (i)Insert");
    printString("\r\n  (W)Save (L)reLoad (+)next (-)prev (Q)uit");
    printString("\r\n  [:]Define [^]Compile [_]Interp [(]Comment");
    printString("\r\n-> \x8");
}

void showEditor() {
    int cp = 0, x = 1, y = 1;
    CursorOff();
    Color(WHITE, 0);
    msg = NULL;
    for (int i = 0; i < NUM_LINES; i++) { showLine(i); }
    showCursor();
    Color(WHITE, 0);
    GotoXY(1, NUM_LINES+1);
}

void deleteChar() {
    for (int o = off; o < (LLEN - 2); o++) {
        edLines[line][o] = edLines[line][o + 1];
    }
    isDirty = 1;
    showLine(line);
    showCursor();
}

void insertChar(char c, int refresh) {
    for (int o = LLEN-1; o > off; o--) {
        edLines[line][o] = edLines[line][o - 1];
    }
    SETC(c);
    isDirty = 1;
    if (refresh) {
        showLine(line);
        showCursor();
    }
}

void doType(int isInsert) {
    CursorOff();
    while (1) {
        char c = key();
        if (c == 27) { return; }
        int isBS = ((c == 127) || (c == 8));
        if (isBS) {
            if (off) {
                --off;
                if (isInsert) { deleteChar(); }
                else { SETC(BL); }
            }
        } else {
            if (isInsert) { insertChar(BL, 0); }
            if (betw(c, 8, 31)) { c = BL; }
            edSetCh(c);
        }
        showLine(line);
        showCursor();
        isDirty = 1;
    }
}

void insertMode() { edMode = INSERT; }
int isCursorMove(int c) { return ((c==9) || betw(c,curLEFT,curEND)) ? 1 : 0; }

int doInsertReplace(char c) {
    if (edMode == INSERT) { insertChar(c, 1); }
    else { edSetCh(c); }
    return 1;
}

int doCR() {
    if (edMode != INSERT) { mv(1,-999); return 1; }
    // TODO: insert a blank line
    mv(1,-999);
    return 1;
}

int moveCursor(int c) {
    switch (c) {
        case 9:        mv(0,8);         break;
        case curLEFT:  mv(0,-1);        break;
        case curRIGHT: mv(0,1);         break;
        case curUP:    mv(-1,0);        break;
        case curDOWN:  mv(1,0);         break;
        case curHOME:  mv(0,-off);      break;
        case curEND:   mv(0,99);        break;
        case curPGUP:  mv(-99,-99);     break;
        case curPGDN:  mv(99,99);       break;
    }
    return 1;
}

int processEditorChar(int c) {
    if (c==27) { edMode = COMMAND; return 1; }
    if (isCursorMove(c)) { return moveCursor(c); }
    if (betw(edMode,INSERT,REPLACE) && betw(c,32,126)) {
        return doInsertReplace((char)c);
    }
    if (c==13) { return doCR(); }
    if (!betw(c,32,126)) { return 1; }
    printChar(c);
    edMode = COMMAND;
    switch (c) {
    case 'Q': toBlock(); return 0;              break;
    case 'i': edMode=INSERT;                    break;
    case 'r': edMode=REPLACE;                   break;
    case 'x': deleteChar();                     break;
    case 'L': edRdBlk();                        break;
    case 'W': edSvBlk();                        break;
    case '+': if (isDirty) { edSvBlk(); }
            ++blkNum;
            edRdBlk();
            break;
    case '-': if (isDirty) { edSvBlk(); }
            blkNum -= (blkNum) ? 1 : 0;
            edRdBlk();
            break;
    }
    return 1;
}

void doEditor(CELL blk) {
    line = 0;
    off = 0;
    blkNum = blk;
    edMode = COMMAND;
    if (0 <= blkNum) { edRdBlk(); }
    blkNum = (0 <= blkNum) ? blkNum : 0;
    CLS();
    showEditor();
    showFooter();
    while (processEditorChar(edKey())) {
        NormLO();
        Color(WHITE, 0);
        showFooter();
    }
    CursorOn();
}

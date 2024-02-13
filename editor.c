// editor.cpp - A simple block editor

#include "cf.h"
#include "block.h"

#define NUM_LINES   20
#define LLEN        (BLOCK_SZ/NUM_LINES)
#define EDCH(l,o)   edBuf[(l*LLEN)+o]
#define DIRTY(l)    isDirty=1; lineShow[l]=1
#define ISCFMODE(c) (BTW(c, RED, WHITE))

enum { COMMAND = 1, INSERT, REPLACE, QUIT };
char *theBlock;
int line, off, blkNum, edMode;
int isDirty, lineShow[NUM_LINES];
char edBuf[BLOCK_SZ], tBuf[LLEN], mode[32], *msg = NULL;
char yanked[LLEN];

void GotoXY(int x, int y) { printStringF("\x1B[%d;%dH", y, x); }
void CLS() { printString("\x1B[2J"); GotoXY(1, 1); }
void ClearEOL() { printString("\x1B[K"); }
void CursorOn() { printString("\x1B[?25h"); }
void CursorOff() { printString("\x1B[?25l"); }
void Color(int c, int bg) { printStringF("\x1B[%d;%dm", (30+c), bg?bg:40); }
void commandMode() { edMode=COMMAND; strCpy(mode, "command"); }
void insertMode()  { edMode=INSERT;  strCpy(mode, "insert"); }
void replaceMode() { edMode=REPLACE; strCpy(mode, "replace"); }
int edKey() { return key(); }

void NormLO() {
    line = min(max(line, 0), NUM_LINES-1);
    off = min(max(off,0), LLEN-1);
}

void showAll() {
    for (int i = 0; i < NUM_LINES; i++) { lineShow[i] = 1; }
}

char edChar(int l, int o, int changeMode) {
    char c = EDCH(l,o);
    if (c==0) { return c; }
    if (ISCFMODE(c) && changeMode) { Color(c, 0); }
    return BTW(c,32,126) ? c : ' ';
}

void showCursor() {
    char c = EDCH(line, off);
    GotoXY(off + 1, line + 1);
    Color(0, 47);
    if (c == 0) c = 'X';
    if (c < 32) c += ('a'-1);
    printChar(c);
}

int getPrevColor(int l) {
    int col=0;
    char *y = &EDCH(l,0);
    while ((col == 0) && (edBuf <= y)) {
        if (BTW(*y,RED,WHITE)) { col=*y; }
        else { --y; }
    }
    return (col) ? col : INTERP;
}

void showLine(int l) {
    if (!lineShow[l]) { return; }
    lineShow[l] = 0;
    Color(getPrevColor(l), 0);
    GotoXY(1, l+1);
    for (int o = 0; o < LLEN; o++) {
        int c = edChar(l, o, 1);
        if (c) { printChar(c); }
        else { ClearEOL(); break; }
    }
    if (l == line) { showCursor(); }
}

void showStatus() {
    static int cnt = 0;
    GotoXY(1, NUM_LINES+1);
    Color(WHITE, 0);
    printString("- Block Editor v0.1 - ");
    printStringF("Block# %03d%s", blkNum, isDirty ? " *" : "");
    printStringF("%s- %s", msg ? msg : " ", mode);
    ClearEOL();
    if (msg && (1 < ++cnt)) { msg = NULL; cnt = 0; }
}

void showHelp() {
    GotoXY(1, NUM_LINES+2);
    Color(WHITE, 0);
    printString("  (h/j/k/l/tab/spc/bs/_/$) Move Cursor (g) Top (G) Bottom");
    printString("\r\n  (x) Del (X) Del-Prev (r) Replace 1 (i) Insert (R) Replace");
    printString("\r\n  (W) Write (L) Reload (+) Next (-) Prev (Q) Quit");
    printString("\r\n  (^A) Define (^B) Comment (^C) Inline (^D) Var");
    printString("\r\n  (^E) Machine (^F) Compile (^G) Interpret");
}

void showEditor() {
    for (int i = 0; i < NUM_LINES; i++) { showLine(i); }
}

void mv(int l, int o) {
    lineShow[line] = 1;
    line += l;
    off += o;
    NormLO();
    lineShow[line] = 1;
}

void gotoEOL() {
    mv(0, -99);
    while (EDCH(line, off) != 10) { ++off; }
}

int toBlock() {
    fill(theBlock, 0, BLOCK_SZ);
    for (int l=0; l < NUM_LINES; l++) {
        char *y=&EDCH(l,0);
        strCat(theBlock,y);
    }
    blockIsDirty(blkNum);
    return strLen(theBlock);
}

void addLF(int l) {
    int o, lc = 0;
    for (o = 0; EDCH(l, o); ++o) { lc = EDCH(l, o); }
    if (lc != 10) { EDCH(l, o) = 10; }
}

void toBuf() {
    int o = 0, l = 0, ch;
    fill(edBuf, 0, BLOCK_SZ);
    for (int i = 0; i < BLOCK_SZ; i++) {
        ch = theBlock[i];
        if (ch == 0) { break; }
        if (ch ==10) {
            EDCH(l, o) = (char)ch;
            if (NUM_LINES <= (++l)) { return; }
            o=0;
            continue;
        } else if ((o < LLEN) && (ch!=13)) {
            EDCH(l,o++) = (char)ch;
        }
    }
    for (int i = 0; i < NUM_LINES; i++) { addLF(i); }
}

void edRdBlk(int force) {
    if (force) { blockClear(blkNum, 0); }
    theBlock = blockRead(blkNum);
    blockIsText(blkNum);
    toBuf();
    showAll();
    isDirty = 0;
}

void edSvBlk(int force) {
    if (isDirty || force) {
        toBlock();
        blockIsDirty(blkNum);
        blockWrite(blkNum, 0);
        isDirty = 0;
    }
}

void deleteChar() {
    for (int o = off; o < (LLEN - 2); o++) {
        EDCH(line,o) = EDCH(line, o+1);
    }
    DIRTY(line);
    addLF(line);
}

void deleteLine() {
    EDCH(line,0) = 0;
    toBlock();
    toBuf();
    showAll();
    isDirty = 1;
}

void insertSpace() {
    for (int o=LLEN-1; off<o; o--) {
        EDCH(line,o) = EDCH(line, o-1);
    }
    EDCH(line,off)=32;
}

void insertLine() {
    insertSpace();
    EDCH(line, off)=10;
    toBlock();
    toBuf();
    showAll();
    // mv(1,-99);
    isDirty = 1;
}

void joinLines() {
    gotoEOL();
    EDCH(line, off) = 0;
    toBlock();
    toBuf();
    showAll();
    isDirty = 1;
}

void replaceChar(char c, int force, int mov) {
    if (!BTW(c, 32, 126) && (!ISCFMODE(c)) && (!force)) { return; }
    for (int o=off-1; 0<=o; --o) {
        int ch = EDCH(line, o);
        if (ch && (ch != 10)) { break; }
        EDCH(line,o)=32;
    }
    EDCH(line, off)=c;
    DIRTY(line);
    addLF(line);
    if (mov) { mv(0, 1); }
}

int doInsertReplace(char c) {
    if (c==13) {
        if (edMode == REPLACE) { mv(1, -999); }
        else { insertLine(); mv(1,-99); }
        return 1;
    }
    if (!BTW(c,32,126) && (!ISCFMODE(c))) { return 1; }
    if (edMode == INSERT) { insertSpace(); }
    replaceChar(c, 1, 1);
    return 1;
}

int doCommon(int c) {
    int l = line, o = off;
    if (c == 8) { mv(0, -1); }                     // <ctrl-h>
    else if (c ==  9) { mv(0, 8); }                // <tab>
    else if (c == 10) { mv(1, 0); }                // <ctrl-j>
    else if (c == 11) { mv(-1, 0); }               // <ctrl-k>
    else if (c == 12) { mv(0, 1); }                // <ctrl-l>
    else if (c == 24) { mv(0, -1); deleteChar(); } // <ctrl-x>
    return ((l!=line) || (o!=off)) ? 1 : 0;
}

int doCTL(int c) {
    if (c==13) { mv(1,-999); }
    else if (BTW(c,RED,WHITE)) {
        if (edChar(line, off, 0)!=' ') {  insertSpace(); }
        replaceChar(c, 1, 0);
    }
    return 1;
}

int processEditorChar(int c) {
    if (c==27) { commandMode(); return 1; }
    if (doCommon(c)) { return 1; }
    if (BTW(edMode,INSERT,REPLACE)) {
        return doInsertReplace((char)c);
    }
    if (c<32) { return doCTL(c); }

    switch (c) {
    case  ' ': mv(0, 1);
    BCASE 'h': mv(0,-1);
    BCASE 'l': mv(0,1);
    BCASE 'j': mv(1,0);
    BCASE 'k': mv(-1,0);
    BCASE '_': mv(0,-99);
    BCASE 'a': mv(0, 1); insertMode();
    BCASE 'A': gotoEOL(); insertMode();
    BCASE 'J': joinLines();
    BCASE '$': gotoEOL();
    BCASE 'g': mv(-99,-99);
    BCASE 'G': mv(99,-999);
    BCASE 'i': insertMode();
    BCASE 'I': mv(0, -99); insertMode();
    BCASE 'o': mv(1, -99); insertLine(); insertMode();
    BCASE 'O': mv(0, -99); insertLine(); insertMode();
    BCASE 'r': replaceChar(edKey(), 0, 1);
    BCASE 'R': replaceMode();
    BCASE 'c': deleteChar(); insertMode();
    BCASE 'C': c=off; while (c<LLEN) { EDCH(line, c++) = 0; }
            addLF(line); isDirty=lineShow[line]=1; insertMode();
    BCASE 'D': deleteLine();
    BCASE 'x': deleteChar();
    BCASE 'X': if (0 < off) { --off; deleteChar(); }
    BCASE 'L': edRdBlk(1);
    BCASE 'W': isDirty = 1; edSvBlk(1);
    BCASE 'Y': strCpy(yanked, &EDCH(line, 0));
    BCASE 'p': mv(1,-99); insertLine(); strCpy(&EDCH(line,0), yanked);
    BCASE 'P': mv(0,-99); insertLine(); strCpy(&EDCH(line,0), yanked);
    BCASE '+': edSvBlk(0); ++blkNum; edRdBlk(0);
    BCASE '-': edSvBlk(0); blkNum = max(0, blkNum-1); edRdBlk(0);
    BCASE 'Q': toBlock(); edMode = QUIT;
    }
    return 1;
}

void doEditor(cell_t blk) {
    blkNum = max((int)blk, 0);
    line = off = 0;
    msg = NULL;
    CLS();
    CursorOff();
    edRdBlk(0);
    commandMode();
    showAll();
    showHelp();
    while (edMode != QUIT) {
        showEditor();
        showStatus();
        processEditorChar(edKey());
    }
    GotoXY(1, NUM_LINES + 7);
    CursorOn();
}

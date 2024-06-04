// editor.cpp - A simple block editor

#include "cf.h"

#define __EDITOR__

#ifdef __EDITOR__

#define LLEN           100
#define NUM_LINES      (BLOCK_SZ/LLEN)
#define MAX_LINE       ((BLOCK_SZ/LLEN)-1)
#define LO2pos(L,O)    ((LLEN*(L))+(O))
#define pos2Line(P)    ((P)/LLEN)
#define pos2Offset(P)  ((P)%LLEN)
#define EDCH(L,O)      theBlock[LO2pos(L,O)]
#define POSCH(x)       theBlock[(x)]
#define DIRTY()        blockDirty(blkNum,1)
#define ISDIRTY()      blockDirtyQ(blkNum)
#define ISCFMODE(c)    (btwi(c, RED, WHITE))

#ifndef min
#define min(a,b)    ((a)<(b))?(a):(b)
#define max(a,b)    ((b)<(a))?(a):(b)
#endif

enum { NORMAL = 1, INSERT, REPLACE, QUIT };
char *theBlock;
int line, off, blkNum, edMode, pos;
char tBuf[LLEN], mode[32], *msg = NULL;
char yanked[LLEN];

void GotoXY(int x, int y) { printStringF("\x1B[%d;%dH", y, x); }
void CLS() { printString("\x1B[2J"); GotoXY(1, 1); }
void ClearEOL() { printString("\x1B[K"); }
void CursorShape(int n) { printStringF("\x1B[%d q", n); }
void CursorOn() { printString("\x1B[?25h"); }
void CursorOff() { printString("\x1B[?25l"); }
void normalMode() { edMode=NORMAL; strCpy(mode, "normal"); }
void insertMode()  { edMode=INSERT;  strCpy(mode, "insert"); }
void replaceMode() { edMode=REPLACE; strCpy(mode, "replace"); }
int  edKey() { return key(); }

void NormLO() {
    pos = min(max(pos, 0), BLOCK_SZ-1);
    line = pos2Line(pos);
    off = pos2Offset(pos);
}

void showCursor() {
    GotoXY(off + 1, line + 1);
    CursorOn();
}

void showStatus() {
    static int cnt = 0;
    GotoXY(1, NUM_LINES+1);
    Color(WHITE, 0);
    printString("- Block Editor v0.1 - ");
    printStringF("Block# %03d%s", blkNum, ISDIRTY() ? " *" : "");
    printStringF("%s- %s", msg ? msg : " ", mode);
    ClearEOL();
    if (msg && (1 < ++cnt)) { msg = NULL; cnt = 0; }
}

void showHelp() {
    GotoXY(1, NUM_LINES+1);
    Color(WHITE, 0);
    printString("\r\n  (^A) Define (^B) Compile (^C) Interpret (^D) N/A");
    printString("\r\n  (^E) N/A (^F) N/A (^G) Comment");
}

void showEditor() {
    int x = 0;
    GotoXY(1,1);
    for (int i = 0; i < NUM_LINES; i++) {
        for (int j = 0; j < LLEN; j++) {
            int c = theBlock[x++];
            if (ISCFMODE(c)) { Color(c, 0); }
            c = btwi(c,33,126) ? c : 32;
            printChar(c);
        }
        printString("\r\n");
    }
}

void mv(int l, int o) {
    pos = LO2pos(line+l, off+o);
    NormLO();
}

void gotoEOL() {
    off = LLEN-1;
    while ((0<=off) && (EDCH(line, off) < 33)) { --off; }
}

void edRdBlk(int force) {
    if (force) { blockReload(blkNum); }
    theBlock = (char*)blockData(blkNum);
    for (int p = 0; p < BLOCK_SZ; p++) { if ((POSCH(p) == 0) || (POSCH(p) == 10)) { POSCH(p) = 32; } }
}

void edSvBlk(int force) {
    if ((ISDIRTY()==0) && (force==0)) { return; }
    // Try to clean up the block for standard text editors
    for (int p = 0; p < BLOCK_SZ; p++) { if ((POSCH(p) == 0) || (POSCH(p) == 10)) { POSCH(p) = 32; } }
    for (int ln = 0; ln < NUM_LINES; ln++) {
        for (int o = LLEN-1; 0 <= o; o--) { if (EDCH(ln,o)==32) { EDCH(ln,o)=10; break; } }
    }
    DIRTY();
}

void deleteChar() {
    int x = LO2pos(line, off);

    for (int o=off; o<(LLEN-1); o++) {
        POSCH(x) = POSCH(x+1); x++;
    }
    POSCH(x) = 32;
    DIRTY();
}

void deleteLine() {
    int f = LO2pos(line, 0);
    int t = LO2pos(line+1, 0);
    while (t < BLOCK_SZ) { POSCH(f++) = POSCH(t++); }
    f = LO2pos(NUM_LINES-1,0);
    for (t=0; t<LLEN; t++) { POSCH(f++) = 32; }
    DIRTY();
}

void insertSpace() {
    for (int o=BLOCK_SZ-1; pos<o; o--) {
        POSCH(o) = POSCH(o-1);
    }
    POSCH(pos)=32;
}

void insertLine() {
    int f = LO2pos(line, 0);
    int t = BLOCK_SZ - 1;
    while (f < t) { POSCH(t) = POSCH(t - LLEN); t--; }
    for (f=0; f<LLEN; f++) { EDCH(line,f) = 32; }
    DIRTY();
}

void joinLines() {
    DIRTY();
}

void replaceChar(char c, int force, int mov) {
    if (!btwi(c, 32, 126) && (!ISCFMODE(c)) && (!force)) { return; }
    int o = pos;
    while ((0<=o) && (POSCH(o) == 0)) { POSCH(o--)=32; }
    POSCH(pos)=c;
    DIRTY();
    if (mov) { mv(0, 1); }
}

int doInsertReplace(char c) {
    if (c==13) {
        if (edMode == REPLACE) { mv(1, -off); }
        else { insertLine(); mv(1,-off); }
        return 1;
    }
    if (!btwi(c,32,126) && (!ISCFMODE(c))) { return 1; }
    if (edMode == INSERT) { insertSpace(); }
    replaceChar(c, 1, 1);
    return 1;
}

int doCommand() {
    GotoXY(NUM_LINES+3,0);
    printChar(':');
    return 1;
}

int doCommon(int c) {
    int l = line, o = off;
    if (c == 8) { mv(0, -1); }                     // <ctrl-h>
    else if (c ==  9) { mv(0, 10); }               // <tab>
    else if (c == 10) { mv(1, 0); }                // <ctrl-j>
    else if (c == 11) { mv(-1, 0); }               // <ctrl-k>
    else if (c == 12) { mv(0, 1); }                // <ctrl-l>
    else if (c == 13) { mv(1, -off); }             // <ctrl-m>
    else if (c == 24) { mv(0, -1); deleteChar(); } // <ctrl-x>
    else if (c == 26) { normalMode(); return 1; }  // <ctrl-z>
    else if (c == 27) { normalMode(); return 1; }  // <escape>
    return ((l!=line) || (o!=off)) ? 1 : 0;
}

int doCTL(int c) {
    if (ISCFMODE(c)) {
        if (32<EDCH(line, off)) {  insertSpace(); }
        replaceChar(c, 1, 0);
    } else {
        doCommon(c);
    }
    return 1;
}

void yankLine(int L) {
    for (int x=0; x<LLEN; x++) {
        yanked[x] = EDCH(L,x);
    }
}

void pasteLine(int L) {
    for (int x=0; x<LLEN; x++) {
        EDCH(L,x) = yanked[x];
    }
}

int processEditorChar(int c) {
    if (c<32) { return doCTL(c); }
    if (btwi(edMode,INSERT,REPLACE)) {
        return doInsertReplace((char)c);
    }

    switch (c) {
    case  ' ': mv(0,  1);
    BCASE 'h': mv(0, -1);
    BCASE 'l': mv(0,  1);
    BCASE 'j': mv(1,  0);
    BCASE 'k': mv(-1, 0);
    BCASE '_': mv(0,-off);
    BCASE ':': doCommand();
    BCASE 'a': mv(0, 1); insertMode();
    BCASE 'A': gotoEOL(); mv(0,1); insertMode();
    BCASE 'J': joinLines();
    BCASE 'g': mv(-line,-off);
    BCASE 'G': mv(99,99);
    BCASE 'i': insertMode();
    BCASE 'I': mv(0, -off); insertMode();
    BCASE 'o': mv(1, -off); insertLine(); insertMode();
    BCASE 'O': mv(0, -off); insertLine(); insertMode();
    BCASE 'r': replaceChar(edKey(), 0, 1);
    BCASE 'R': replaceMode();
    BCASE 'c': deleteChar(); insertMode();
    BCASE 'C': c=off; while (c<LLEN) { EDCH(line, c++) = 32; }
            DIRTY(); insertMode();
    BCASE 'D': yankLine(line); deleteLine();
    BCASE 'x': deleteChar();
    BCASE 'X': if (0 < off) { mv(0, -1); deleteChar(); }
    BCASE 'L': edRdBlk(1);
    BCASE 'W': DIRTY(); edSvBlk(1);
    BCASE 'Y': yankLine(line);
    BCASE 'p': mv(1,-off); insertLine(); pasteLine(line);
    BCASE 'P': mv(0,-off); insertLine(); pasteLine(line);
    BCASE '+': edSvBlk(0); ++blkNum; edRdBlk(0);
    BCASE '-': edSvBlk(0); blkNum = max(0, blkNum-1); edRdBlk(0);
    BCASE 'Q': edMode = QUIT;
    }
    return 1;
}

void doEditor(int blk) {
    blkNum = max((int)blk, 0);
    line = off = pos = 0;
    msg = NULL;
    CLS();
    CursorShape(2);
    CursorOff();
    edRdBlk(0);
    normalMode();
    showHelp();
    while (edMode != QUIT) {
        CursorOff();
        NormLO();
        showEditor();
        showStatus();
        showCursor();
        processEditorChar(edKey());
    }
    GotoXY(1, NUM_LINES + 7);
    CursorOn();
    CursorShape(5);
}

#endif // __EDITOR__

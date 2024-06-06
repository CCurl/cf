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

int edKey() {
    // NB: in Windows, <ctrl-h> and <backspace> both return 8
    int k = key();
    if (k==224) {
        // Windows special keys
        switch (key()) {
            case 72: return 11; // <up>
            case 80: return 10; // <down>
            case 77: return 12; // <right>
            case 75: return  8; // <left>
            case 83: return 25; // <delete>
            case 71: return 29; // <home>
            case 79: return 28; // <end>
        }
    }
    return k;
}

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
    if (msg) { printStringF(" - %s", msg); }
    printStringF(" - %s", mode);
    printStringF(" - [%02d:%02d] (#%d/$%X)", line, off, POSCH(pos), POSCH(pos));
    ClearEOL();
    if (msg && (1 < ++cnt)) { msg = NULL; cnt = 0; }
}

void showHelp() {
    GotoXY(1, NUM_LINES+1);
    Color(WHITE, 0);
    printString("\r\n  (^A) Define (^B) Compile (^C) Interpret (^D) Macro (^G) Comment");
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

void edRdBlk(int force) {
    if (force) { blockReload(blkNum); }
    theBlock = (char*)blockData(blkNum);
    for (int p = 0; p < BLOCK_SZ; p++) {
        if ((POSCH(p) == 0) || (POSCH(p) == 10)) { POSCH(p) = 32; }
    }
}

void edSvBlk(int force) {
    if ((ISDIRTY()==0) && (force==0)) { return; }
    // Try to clean up the block for standard text editors
    for (int p = 0; p < BLOCK_SZ; p++) { if ((POSCH(p) == 0) || (POSCH(p) == 10)) { POSCH(p) = 32; } }
    for (int ln = 0; ln < NUM_LINES; ln++) {
        for (int o = LLEN-1; 0 <= o; o--) { if (EDCH(ln,o)==32) { EDCH(ln,o)=10; break; } }
    }
    flushBlock(blkNum, 1);
}

void deleteChar() {
    int x = pos;
    while ((x+1)<BLOCK_SZ) { POSCH(x) = POSCH(x+1); ++x; }
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
    DIRTY();
}

void insertLine() {
    int f = LO2pos(line, 0);
    int t = BLOCK_SZ - 1;
    while (f < t) { POSCH(t) = POSCH(t - LLEN); t--; }
    for (f=0; f<LLEN; f++) { EDCH(line,f) = 32; }
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

int accept(char *buf) {
    int ln = 0;
    while (1) {
        int c = key();
        if ((c==27) || (c==3)) { buf[0]=0; return 0; }
        else if (c==13) { buf[ln]=0; return ln; }
        else if ((c==127) || (c==8)) {
            if (0<ln) { --ln; printStringF("%c %c",8,8); }
        }
        else if (btwi(c,32,126)) { buf[ln++]=c; printChar(c); }
    }
    return ln;
}

int doCommand() {
    char buf[32];
    GotoXY(1, NUM_LINES+3);
    printString(":");
    ClearEOL();
    int ln = accept(buf);
    GotoXY(1, NUM_LINES+3);
    ClearEOL();
    if (strEq(buf,"w"))  { edSvBlk(1); }
    if (strEq(buf,"wq")) { edSvBlk(1); edMode = QUIT; }
    if (strEq(buf,"rl")) { edRdBlk(1); }
    if (strEq(buf,"q!")) { edRdBlk(1); edMode = QUIT; }
    if (strEq(buf,"q")) {
        if (ISDIRTY()) { printString("use q! to quit"); }
        else { edMode = QUIT; } }
    return 1;
}

int doCommon(int c) {
    int l = line, o = off;
    switch (c) {
        case   8:  mv(0, -1);                // <ctrl-h> - left
        BCASE  9:  mv(0,  8);                // <tab>
        BCASE 17:  mv(0, -8);                // <ctrl-q> - tab-left
        BCASE 10:  mv(1, 0);                 // <ctrl-j> - down
        BCASE 11:  mv(-1, 0);                // <ctrl-k> - up
        BCASE 12:  mv(0, 1);                 // <ctrl-l> - right
        BCASE 13:  mv(1, -off);              // <ctrl-m> - <enter>
        BCASE  5:  mv(4,  0);                // <ctrl-e> - down 4
        BCASE 21:  mv(-4, 0);                // <ctrl-u> - up 4
        BCASE 28:  off=LLEN-1; mv(0, 0);     // <ctrl-$> - <end>
        BCASE 29:  mv(0, -off);              // <ctrl-%> - home
        BCASE 24:  mv(0, -1); deleteChar();  // <ctrl-x> - backspace
        BCASE 25:  deleteChar();             // <ctrl-y> - delete
        BCASE 26:  normalMode(); return 1;   // <ctrl-z>
        BCASE 27:  normalMode(); return 1;   // <escape>
    }
    return ((l!=line) || (o!=off)) ? 1 : 0;
}

int doCTL(int c) {
    if (ISCFMODE(c)) {
        if (32<EDCH(line, off)) {  insertSpace(); }
        replaceChar(c, 1, btwi(edMode,INSERT, REPLACE));
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

void deleteToEOL(int c) { while (c < LLEN) { EDCH(line, c++) = 32; } }

void pasteLine(int L) {
    for (int x=0; x<LLEN; x++) {
        EDCH(L,x) = yanked[x];
    }
}

int processEditorChar(int c) {
    if (c==127) { c = 24; }
    if (c < 32) { return doCTL(c); }
    if (btwi(edMode,INSERT,REPLACE)) {
        return doInsertReplace((char)c);
    }

    switch (c) {
        case  ' ': mv(0,  1);
        BCASE 'h': mv(0, -1);
        BCASE 'l': mv(0,  1);
        BCASE 'q': mv(0,  8);
        BCASE 'Q': mv(0, -8);
        BCASE 'j': mv(1,  0);
        BCASE 'k': mv(-1, 0);
        BCASE 'e': mv(4, 0);
        BCASE 'u': mv(-4, 0);
        BCASE '_': mv(0,-off);
        BCASE '$': off = LLEN-1; mv(0,0);
        BCASE ':': doCommand();
        BCASE 'g': mv(-line,-off);
        BCASE 'G': mv(999,999);
        BCASE 'i': insertMode();
        BCASE 'I': mv(0, -off); insertMode();
        BCASE 'o': mv(1, -off); insertLine(); replaceMode();
        BCASE 'O': mv(0, -off); insertLine(); replaceMode();
        BCASE 'r': replaceChar(edKey(), 0, 1);
        BCASE 'R': replaceMode();
        BCASE 'C': deleteToEOL(off); DIRTY(); replaceMode();
        BCASE 'D': yankLine(line); deleteLine();
        BCASE 'x': deleteChar();
        BCASE 'X': if (0 < pos) { mv(0, -1); deleteChar(); }
        BCASE 'z': deleteChar();  c=pos; pos=LO2pos(line, LLEN-2); insertSpace(); pos=c;
        BCASE 'b': insertSpace();
        BCASE 'B': insertSpace(); c=pos; pos=LO2pos(line, LLEN-2); deleteChar();  pos=c;
        BCASE 'L': edRdBlk(1);
        BCASE 'Y': yankLine(line);
        BCASE 'p': mv(1,-off); insertLine(); pasteLine(line);
        BCASE 'P': mv(0,-off); insertLine(); pasteLine(line);
        BCASE '+': edSvBlk(0); ++blkNum; edRdBlk(0); mv(-line, -off);
        BCASE '-': edSvBlk(0); blkNum=max(0,blkNum-1); edRdBlk(0);  mv(-line, -off);
    }
    return 1;
}

void doEditor(int blk) {
    blkNum = max((int)blk, 0);
    line = off = pos = 0;
    msg = NULL;
    CLS();
    CursorShape(2); // 1/2=>box (blink/steady), 3/4=>underline, 5/6=>bar
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
    // CursorShape(5);
}

#endif // __EDITOR__

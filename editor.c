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

enum { LF=8, TABR=9, DN=10, UP=11, RT=12, ENT=13, TABL=17,
        PGUP=21, PGDN=22, BKSP=24, DEL=25, ESC2=26, ESC=27,
        END=28, HOME=29, INS=30 };

void GotoXY(int x, int y) { printStringF("\x1B[%d;%dH", y, x); }
void CLS() { printString("\x1B[2J"); GotoXY(1, 1); }
void ClearEOL() { printString("\x1B[K"); }
void CursorShape(int n) { printStringF("\x1B[%d q", n); }
void CursorOn() { printString("\x1B[?25h"); }
void CursorOff() { printString("\x1B[?25l"); }
void normalMode() { edMode=NORMAL; strCpy(mode, "normal"); }
void insertMode()  { edMode=INSERT;  strCpy(mode, "insert"); }
void replaceMode() { edMode=REPLACE; strCpy(mode, "replace"); }
int insertToggle() { edMode == INSERT ? normalMode() : insertMode(); return 1; }

int edKey() {
    // NB: in Windows, <ctrl-h> and <backspace> both return 8
    int k = key();
    if (k == 224) {
        // Windows special keys
        switch (key()) {
            case 71: return HOME;
            case 72: return UP;
            case 73: return PGUP;
            case 75: return LF;
            case 77: return RT;
            case 79: return END;
            case 80: return DN;
            case 81: return PGDN;
            case 82: return INS;
            case 83: return DEL;
        }
        return 27;
    }
    else if (k == 27) {
        // Possible Linux/VT special key
        if (key() == 91) {
            k = key();
            if (k == 65) { return UP; }
            if (k == 66) { return DN; }
            if (k == 67) { return RT; }
            if (k == 68) { return LF; }
            if (k == 70) { return END; }
            if (k == 72) { return HOME; }
            if (btwi(k,49,54)) {
                if (key() == 126) {
                    if (k == 49) { return HOME; }
                    if (k == 50) { return INS; }
                    if (k == 51) { return DEL; }
                    if (k == 52) { return END; }
                    if (k == 53) { return PGUP; }
                    if (k == 54) { return PGDN; }
                }
            }
        }
        return 27;
    }
    return k;
}

void NormLO() {
    pos = min(max(pos, 0), BLOCK_SZ-1);
    line = pos2Line(pos);
    off = pos2Offset(pos);
}

void mv(int l, int o) {
    pos = LO2pos(line+l, off+o);
    NormLO();
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

void edRdBlk(int force) {
    if (force) { blockReload(blkNum); }
    theBlock = (char*)blockData(blkNum);
    for (int p = 0; p < BLOCK_SZ; p++) {
        int c = POSCH(p);
        if ((c==0) || btwi(c,9,31)) { POSCH(p) = 32; }
    }
}

void edSvBlk(int force) {
    if ((ISDIRTY()==0) && (force==0)) { return; }
    // Try to clean up the block for standard text editors
    for (int p=0; p<BLOCK_SZ; p++) { if (btwi(POSCH(p),9,31)) { POSCH(p)=32; } }
    for (int ln=0; ln < NUM_LINES; ln++) {
        for (int o=LLEN-1; 0<=o; o--) { if (EDCH(ln,o)==32) { EDCH(ln,o)=10; break; } }
    }
    flushBlock(blkNum, 1);
}

void deleteChar(int LineOnly) {
    int x = pos;
    int stopHere = (LineOnly) ? LO2pos(line, LLEN) : BLOCK_SZ;
    while ((x+1)<stopHere) { POSCH(x) = POSCH(x+1); ++x; }
    if (line) { POSCH(x) = 32; }
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

void insertSpace(int lineOnly) {
    int o = (lineOnly) ? LO2pos(line,LLEN-1) : BLOCK_SZ-1;
    while(pos < o) { POSCH(o) = POSCH(o-1); --o; }
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
    if (edMode == INSERT) { insertSpace(1); }
    replaceChar(c, 1, 1);
    return 1;
}

int accept(char *buf) {
    int ln = 0;
    while (1) {
        int c = key();
        if ((c==27) || (c==3) || (c==26)) { buf[0]=0; return 0; }
        if (c==13) { buf[ln]=0; return ln; }
        if (btwi(c,32,126)) { buf[ln++]=c; printChar(c); }
        if ((c==127) || (c==8)) {
            if (0<ln) { --ln; EMIT(8); EMIT(32); EMIT(8); }
        }
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
        BCASE  9:   mv(0,  8);
        BCASE TABL: mv(0, -8);
        case  LF:   mv(0, -1);
        BCASE RT:   mv(0,  1);
        BCASE UP:   mv(-1, 0);
        BCASE DN:   mv(1,  0);
        BCASE 13:   mv(1, -off);
        BCASE PGUP: mv(-5, 0);
        BCASE PGDN: mv(5,  0);
        BCASE BKSP: mv(0, -1); deleteChar(1);
        BCASE DEL:  deleteChar(1);
        BCASE ESC2: normalMode(); return 1;
        BCASE ESC:  normalMode(); return 1;
        BCASE END:  off=LLEN-1; mv(0, 0);
        BCASE HOME: mv(0, -off);
        BCASE INS:  return insertToggle();
    }
    return ((l!=line) || (o!=off)) ? 1 : 0;
}

int doCTL(int c) {
    if (ISCFMODE(c)) {
        if (32<EDCH(line, off)) {  insertSpace(1); }
        replaceChar(c, 1, btwi(edMode,INSERT, REPLACE));
    } else {
        doCommon(c);
    }
    return 1;
}

void yankLine(int L) { for (int x=0; x<LLEN; x++) { yanked[x] = EDCH(L,x); } }
void deleteToEOL(int o) { while (o < LLEN) { EDCH(line, o++) = 32; } }

void doDelete(int x) {
    if (x==0) { x=key(); }
    switch (x) {
        case '.': deleteChar(1);
        BCASE '$': deleteToEOL(off);
        BCASE 'd': deleteLine();
        BCASE '_': while (off) { --off; pos=LO2pos(line,off); deleteChar(1); }
    }
}

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
        BCASE 'G': line = NUM_LINES-1; off = 0; mv(0, 0);
        BCASE 'i': insertMode();
        BCASE 'I': mv(0, -off); insertMode();
        BCASE 'o': mv(1, -off); insertLine(); replaceMode();
        BCASE 'O': mv(0, -off); insertLine(); replaceMode();
        BCASE 'r': replaceChar(edKey(), 0, 1);
        BCASE 'R': replaceMode();
        BCASE 'C': deleteToEOL(off); DIRTY(); replaceMode();
        BCASE 'd': doDelete(0);
        BCASE 'D': yankLine(line); deleteLine();
        BCASE 'x': deleteChar(1);
        BCASE 'X': deleteChar(0);
        BCASE 'H': mv(0, -1); deleteChar(1);
        BCASE 'b': insertSpace(1);
        BCASE 'B': insertSpace(0);
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

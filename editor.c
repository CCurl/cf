// editor.cpp - A simple block editor

#include "cf.h"

#define COMMAND       1
#define INSERT        2
#define REPLACE       3
#define QUIT         99
#define CELL cell_t

#define LLEN       100
#define NUM_LINES   20
#define BLOCK_SZ    (NUM_LINES)*(LLEN)
#define MAX_CUR     (BLOCK_SZ-1)
#define EDCH(l,o)   edBuf[(l*LLEN)+o]
#define SETC(c)     EDCH(line,off)=c

char theBlock[BLOCK_SZ];
int line, off, blkNum, edMode;
int isDirty = 0, currentColor=WHITE;
char mode[32], *msg = NULL;
char edBuf[BLOCK_SZ], tBuf[LLEN];

void GotoXY(int x, int y) { printStringF("\x1B[%d;%dH", y, x); }
void CLS() { printString("\x1B[2J"); GotoXY(1, 1); }
void ClearEOL() { printString("\x1B[K"); }
void CursorOn() { printString("\x1B[?25h"); }
void CursorOff() { printString("\x1B[?25l"); }
void Color(int c, int bg) { printStringF("\x1B[%d;%dm", (30+c), bg?bg:40); }
void commandMode() { edMode = COMMAND; strCpy(mode, "command"); }
void fillWith(char* x, int num, byte ch) { for (int i = 0; i < num; i++) x[i] = ch; }
int edKey() { return key(); }

void NormLO() {
    line = min(max(line, 0), NUM_LINES-1);
    off = min(max(off,0), LLEN-1);
}

char edChar(int l, int o, int changeMode) {
    char c = EDCH(l,o);
    if (BTW(c, RED, WHITE) && changeMode) {
        Color(c, 0); currentColor = c;
    }
    return BTW(c,32,126) ? c : ' ';
}

void showLine(int l) {
    GotoXY(1, l + 1);
    for (int o = 0; o < LLEN; o++) {
        printChar(edChar(l, o, 1));
    }
}

void showEditor() {
    for (int i = 0; i < NUM_LINES; i++) { showLine(i); }
}

void showCursor() {
    NormLO();
    char c = edChar(line, off, 0);
    GotoXY(off + 1, line + 1);
    Color(0, 47);
    printChar(c ? c : 'X');
    Color(currentColor, 0);
}

void mv(int l, int o) {
    if (l) { showLine(line); }
    line += l;
    off += o;
    NormLO();
    showLine(line);
}

void edSetCh(char c, int move) {
    SETC(c);
    if (move) { mv(0, 1); }
    isDirty = 1;
}

int toBlock() {
    int c = 0;
    fillWith(theBlock, BLOCK_SZ, 0);
    for (int l = 0; l < NUM_LINES; l++) {
        for (int o = 0; o < LLEN; o++) { theBlock[c++] = EDCH(l,o); }
        int o=LLEN-1;
        while ((theBlock[c-1] < 33) && (0 <= o)) { theBlock[--c]=0; --o; }
        theBlock[c++]=10;
    }
    return c;
}

void toBuf() {
    int o = 0, l = 0, ch;
    fillWith(edBuf, BLOCK_SZ, 32);
    for (int i = 0; i < BLOCK_SZ; i++) {
        ch = theBlock[i];
        if (ch == 0) { return; }
        if (ch ==10) {
            if (NUM_LINES <= (++l)) { return; }
            o=0; continue;
        } else if ((o < LLEN) && (ch!=13)) {
            EDCH(l,o++) = (char)ch;
        }
    }
}

void edRdBlk() {
    char buf[24];
    fillWith(theBlock, BLOCK_SZ, 0);
    sprintf(buf, "block-%03d.cf", blkNum);
    msg = " - noFile ";
    FILE* fp = fopen(buf, "rb");
    if (fp) {
        int n = (int)fread(theBlock, 1, BLOCK_SZ, fp);
        msg = tBuf;
        sprintf(msg, " - loaded (%d chars) ", n);
        fclose(fp);
    }
    toBuf();
    isDirty = 0;
    showEditor();
}

void edSvBlk() {
    int sz = toBlock();
    char buf[24];
    sprintf(buf, "block-%03d.cf", blkNum);
    msg = " - err ";
    FILE* fp = fopen(buf, "wb");
    if (fp) {
        fwrite(theBlock, 1, sz, fp);
        msg = " - saved ";
        fclose(fp);
    }
    isDirty = 0;
}

void showFooter() {
    static int cnt=0;
    showCursor();
    GotoXY(1, NUM_LINES+1);
    Color(WHITE, 0);
    printString("- Block Editor v0.1 - ");
    printStringF("Block# %03d%s", blkNum, isDirty ? " *" : "");
    printStringF("%s- %s", msg ? msg : " ", mode);
    ClearEOL();
    if (msg && (1<++cnt)) { msg=NULL; cnt=0; }
    printString("\r\n  (h/j/k/l/tab/spc/bs/_/$) Move Cursor (g) Top (G) Bottom");
    printString("\r\n  (x) Del (X) Del-Prev (r) Replace 1 (i) Insert (R) Replace");
    printString("\r\n  (W) Write (L) Reload (+) Next (-) Prev (Q) Quit");
    printString("\r\n  (^A) Define (^B) Comment (^C) Inline (^D) Var");
    printString("\r\n  (^E) Machine (^F) Compile (^G) Interpret");
    printString("\r\n-> \x8");
    ClearEOL();
}

void deleteChar() {
    for (int o = off; o < (LLEN - 2); o++) {
        EDCH(line,o) = EDCH(line, o+1);
    }
    isDirty = 1;
    showLine(line);
}

void insertSpace() {
    for (int o = LLEN-1; o > off; o--) {
        EDCH(line,o) = EDCH(line, o-1);
    }
    SETC(32);
}

void replaceChar(char c, int force, int mov) {
    if ((c<32) && (force==0)) { return; }
    SETC(c);
    isDirty = 1;
    if (mov) { mv(0, 1); }
    else { showLine(line); }
}

int doInsertReplace(char c, int force) {
    if (c == 8) { mv(0, -1); return 1; }
    if (c == 13) { mv(1, -999); return 1; }
    if (!BTW(c,32,126) && (!force)) { return 1; }
    if (edMode == INSERT) { insertSpace(); }
    replaceChar(c, 0, 1);
    return 1;
}

int doCTL(int c) {
    if (c==8) { mv(0,-1); }
    else if (c==9) { mv(0,8); }
    else if (c==13) { mv(1,-999); }
    else if (BTW(c,RED,WHITE)) {
        if (edChar(line, off, 0)!=' ') {  insertSpace(); }
        replaceChar(c, 1, 0);
    }
    return 1;
}

int processEditorChar(int c) {
    if (c==27) { commandMode(); return 1; }
    if (BTW(edMode,INSERT,REPLACE)) {
        return doInsertReplace((char)c, 0);
    }
    if (c<32) { return doCTL(c); }

    switch (c) {
    case  'h': mv(0,-1);
    BCASE ' ': mv(0, 1);
    BCASE 'l': mv(0,1);
    BCASE 'j': mv(1,0);
    BCASE 'k': mv(-1,0);
    BCASE '_': mv(0,-99);
    BCASE '$': mv(0,99);
    BCASE 'g': mv(-99,-99);
    BCASE 'G': mv(99,-999);
    BCASE 'i': edMode=INSERT; strCpy(mode, "insert");
    BCASE 'r': printString(" replace ch.."); replaceChar(edKey(), 0, 1);
    BCASE 'R': edMode=REPLACE; strCpy(mode, "replace");
    BCASE 'x': deleteChar();
    BCASE 'X': if (0 < off) { --off; deleteChar(); }
    BCASE 'L': edRdBlk();
    BCASE 'W': edSvBlk();
    BCASE '+': if (isDirty) { edSvBlk(); }
            blkNum = min(999,blkNum+1); edRdBlk();
    BCASE '-': if (isDirty) { edSvBlk(); }
            blkNum = max(0, blkNum-1); edRdBlk();
    BCASE 'Q': toBlock(); edMode = QUIT;
    }
    return 1;
}

void doEditor(CELL blk) {
    blkNum = max((int)blk, 0);
    line = off = 0;
    msg = NULL;
    CLS();
    CursorOff();
    edRdBlk();
    commandMode();
    while (edMode != QUIT) {
        showFooter();
        processEditorChar(edKey());
    }
    CursorOn();
}

// cf.c - A ColorForth inspired system

#include "cf.h"

#if _MSC_VER
#include <conio.h>
int key() { return _getch(); }
int qKey() { return _kbhit(); }
#elif IS_LINUX
#include "linux.inc"
#endif

void printString(const char *s) { printf("%s", s); }
void printChar(char c) { printf("%c", c); }

#ifdef NEEDS_ALIGN
cell_t Fetch(const byte *a) {
    cell_t x = *(a++);
    x = (x<<8) | *(a++);
    x = (x<<8) | *(a++);
    x = (x<<8) | *(a);
}

void Store(const byte *a, cell_t v) {
    *(a++) = (v%0xff); v=v>>8;
    *(a++) = (v%0xff); v=v>>8;
    *(a++) = (v%0xff); v=v>>8;
    *(a) = (v%0xff);
}
#else
cell_t Fetch(const char *a) { return *(cell_t*)(a); }
void Store(const char *a, cell_t v) { *(cell_t*)(a) = v; }
#endif

char *doUser(char *pc, char ir) {
    if (ir==100) {
        char fn[32];
        doEditor(pop());
    }
    return pc;
}
cell_t sysTime() { return (cell_t)clock(); }

int checkState(int c, int set) {
    if (BTW(c,1,7) && (c!=PURPLE)) {
        if (set) { state=c; }
        //printStringF("-chSt:%d-",c);
        return c;
    }
    return 0;
}

int getWord(char *wd) {
    int len = 0;
    if (DSP < 0) { printString("-under-"); DSP=0; }
    if (STK_SZ < DSP) { printString("-over-"); DSP=STK_SZ; }
    while (*in && (*in < 33)) { checkState(*(in++), 1); }
    while (32 < *in) { wd[len++] = *(in++); }
    wd[len] = 0;
    return len;
}

int doCompile(const char* wd) {
    // printStringF("-com:%s-", wd);
    if (isNum(wd)) {
        cell_t val = pop();
        if (BTW(val,0,127)) {
            CComma(LIT1);
            CComma(val);
        } else {
            CComma(LIT);
            Comma(val);
        }
        return 1;
    }
    if (doFind(wd)) {
        cell_t f = pop();
        cell_t xt = pop();
        if (f == 2) {    // INLINE
            byte *x=(byte *)xt;
            // printf("-inl-%d-",*x);
            CComma(*x++);
            while (*x != EXIT) { CComma(*(x++)); }
        } else {
            CComma(CALL);
            Comma(xt);
        }
        return 1;
    }
	printStringF("-C:%s?-", wd);
    return 0;
}

int doInterpret(const char* wd) {
    // printStringF("-interp:%s-", wd);
    if (isNum(wd)) { return 1; }
    if (doFind(wd)) {
        char *cp = here;
        cell_t f=pop();
        cell_t xt=pop();
        //printf("-I:%s/%lx-",wd,xt);
        CComma(CALL);
        Comma(xt);
        CComma(EXIT);
        here = cp;
        Run(here);
        return 1;
    }
	printStringF("-I:%s?-", wd);
    return 0;
}

int doML(char *wd) {
	if (isNum(wd)) { CComma(pop()); return 1; }
	printStringF("-M:%s?-", wd);
	return 0;
}

cell_t lastState;
int setState(char *wd) {
    if (strEq(wd, "((")) { state=COMMENT; return 1; }
    if (strEq(wd, "::")) { state=DEFINE; return 1; }
    if (strEq(wd, ":I")) { state=INLINE; return 1; }
    if (strEq(wd, "]]")) { state=COMPILE; return 1; }
    if (strEq(wd, "[[")) { state=INTERP;  return 1; }
    if (strEq(wd, ":M")) { state=MLMODE; return 1; }
    return 0;
}

void doOuter(char* cp) {
    char buf[32];
    in = cp;
    while (getWord(buf)) {
        if (setState(buf)) { continue; }
        switch (state) {
        case COMMENT:
        BCASE DEFINE:  doDefine(buf);
        BCASE INLINE:  doDefine(buf); last->f=2;
        BCASE COMPILE: if (!doCompile(buf)) return;
        BCASE INTERP:  if (!doInterpret(buf)) return;
        BCASE MLMODE:  if (!doML(buf)) return;
        break; default: printStringF("-state?-"); break;
        }
    }
}

void parseF(char *fmt, ...) {
    char *buf = (char*)last;
    buf -= 256;
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, 200, fmt, args);
    va_end(args);
    doOuter(buf);
}

void initVM() {
    vmInit();
    state = INTERP;
    char *cni = ":I %s ]] #%ld ;";
    char *cnn = ":: %s ]] #%zu ;";
    char *m1i = ":I %s :M #%ld 3";
    char *m2i = ":I %s :M #%ld #%ld 3";

    parseF(m1i, ";", EXIT);
    parseF(m1i, "@", FETCH);
    parseF(m1i, "C@", CFETCH);
    parseF(m1i, "!", STORE);
    parseF(m1i, "C!", CSTORE);
    parseF(m1i, "+", ADD);
    parseF(m1i, "-", SUB);
    parseF(m1i, "*", MULT);
    parseF(m1i, "/MOD", SLMOD);
    parseF(m1i, "DUP", DUP);
    parseF(m1i, "SWAP", SWAP);
    parseF(m1i, "DROP", DROP);
    parseF(m1i, "DO", DO);
    parseF(m1i, "LOOP", LOOP);
    parseF(m2i, "(.)", SYS_OPS, DOT);
    parseF(m2i, "EMIT", SYS_OPS, EMIT);
    parseF(m2i, "CLOCK", SYS_OPS, TIMER);
    parseF(m2i, ",", SYS_OPS, COMMA);
    parseF(m2i, "c,", SYS_OPS, CCOMMA);
    parseF(m1i, "edit", 100);

    parseF(cni, "cell", CELL_SZ);
    parseF(cnn, "(here)",(cell_t)&here);
    parseF(cnn, "(last)",(cell_t)&last);
    parseF(cnn, "state",(cell_t)&state);
    parseF(cnn, "base",(cell_t)&base);
    parseF(cnn, "vars",(cell_t)&vars[0]);
    parseF(cnn, "vars-end",(cell_t)&vars[VARS_SZ]);
    doOuter(":I NIP ]] SWAP DROP ;");
    doOuter(":I /   ]] /MOD NIP ;");
    doOuter(":: bye ]] 999 state ! ;");
}

void loop() {
    cell_t fp = input_fp;
    char buf[128];
    if (fp == 0) { fp = (cell_t)stdin; }
    if (fp == (cell_t)stdin) { printString(" ok\r\n"); state=INTERP; }
    if (fgets(buf, 128, (FILE *)fp) == buf) { doOuter(buf); }
    else {
        if (input_fp == (cell_t)stdin) { state = 999; }
        else { fclose((FILE*)input_fp); }
        input_fp = 0;
    }
}

int main(int argc, char **argv) {
    initVM();
    input_fp = (cell_t)fopen("block-000.cf","rt");
    while (state != 999) { loop(); }
    return 1;
}

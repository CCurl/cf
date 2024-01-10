// cf.c - A ColorForth inspired system

#include "cf.h"

#if _MSC_VER
#include <conio.h>
int key() { return _getch(); }
int qKey() { return _kbhit(); }
#elif IS_LINUX
#include "linux.inc"
#endif

enum { EDIT=100, BLOAD, FOPEN, FCLOSE, FREAD, FWRITE, WORDS };

void printString(const char *s) { fprintf((FILE*)output_fp, "%s", s); }
void printChar(char c) { fputc(c, (FILE*)output_fp); }
cell_t sysTime() { return (cell_t)clock(); }
int changeState(int st) { state = st; return st; }

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

void doWords() {
    dict_t *dp = last;
    int n = 0;
    while (dp < (dict_t*)&code[CODE_SZ]) {
        if (10<n) { printString("\r\n"); n = 0; }
        printString(dp->name);
        printChar(9);
        ++n;
        if (8 < dp->len) { ++n; }
        ++dp;
    }
}

char *doUser(char *pc, char ir) {
    char *c1, *c2;
    cell_t n1, n2;
    switch (ir) {
        case  EDIT:   doEditor(pop());
        RCASE BLOAD:  c1=in; doOuter(blockRead(pop())); in=c1;
        RCASE FOPEN:  c2=cpop(); c1=cpop(); push((cell_t)fopen(c1,c2));
        RCASE FCLOSE: c1=cpop(); fclose((FILE*)c1);
        RCASE FREAD:  c2=cpop(); n2=pop(); n1=pop(); c1=cpop();
                push((cell_t)fread(c1, n1, n2, (FILE*)c2));
        RCASE FWRITE: c2=cpop(); n2=pop(); n1=pop(); c1=cpop();
                push((cell_t)fwrite(c1, n1, n2, (FILE*)c2));
        RCASE WORDS:  doWords();
        return pc; default: break;
    }
    return pc;
}

int checkState(int c, int isSet) {
    if (!BTW(c, RED, WHITE)) { return 0; }
    return (isSet) ? changeState(c) : c;
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
            CComma(LIT1); CComma(val);
        } else {
            CComma(LIT); Comma(val);
        }
        return 1;
    }
    t1 = isRegOp(wd);
    if (t1) {
        CComma(t1); CComma(wd[1] - '0');
        return 1;
    }
    if (doFind(wd)) {
        cell_t f = pop();
        cell_t xt = pop();
        if (f == IS_INLINE) {
            byte *x=(byte *)xt;
            // printf("-inl-%d-",*x);
            CComma(*x++);
            while (*x != EXIT) { CComma(*(x++)); }
        } else {
            CComma(CALL); Comma(xt);
        }
        return 1;
    }
	printStringF("-C:%s?-", wd);
    return 0;
}

int doInterpret(const char* wd) {
    // printStringF("-interp:%s-", wd);
    if (isNum(wd)) { return 1; }
    t1 = isRegOp(wd);
    if (t1) {
        *(here) = (char)t1;
        *(here+1) = wd[1]-'0';
        *(here+2) = EXIT;
        Run(here);
        return 1;
    }
    if (doFind(wd)) {
        char *cp = here;
        cell_t f=pop();
        cell_t xt=pop();
        //printf("-I:%s/%lx-",wd,xt);
        CComma(CALL); Comma(xt); CComma(EXIT);
        here = cp; Run(here);
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

int setState(char *wd) {

    if (strEq(wd, "((")) { return changeState(COMMENT); }
    if (strEq(wd, ":D")) { return changeState(DEFINE);  }
    if (strEq(wd, ":I")) { return changeState(INLINE);  }
    if (strEq(wd, "]]")) { return changeState(COMPILE); }
    if (strEq(wd, "[[")) { return changeState(INTERP);  }
    if (strEq(wd, ":M")) { return changeState(MLMODE);  }

    // Auto state transitions for text-based usage
    // static int lastState=0;
    // if ((lastState) && (state==COMMENT) && !strEq(wd, ")")) { return 1; }
    // if (strEq(wd, ":"))  { doDefine(0); return changeState(COMPILE); }
    // if (strEq(wd, ":i")) { doDefine(0); last->f=2; return changeState(COMPILE); }
    // if (strEq(wd, ":m")) { doDefine(0); last->f=2; return changeState(MLMODE); }
    // if (strEq(wd, "["))  { return changeState(INTERP); }
    // if (strEq(wd, "]"))  { return changeState(COMPILE); }
    // if (strEq(wd, "("))  { lastState=(int)state; return changeState(COMMENT); }
    // if (strEq(wd, ")"))  { int x=lastState; lastState=0; return changeState(x); }
    return 0;
}

void doOuter(const char *src) {
    char buf[32];
    in = (char*)src;
    while (getWord(buf)) {
        if (setState(buf)) { continue; }
        switch (state) {
            case COMMENT:
            BCASE DEFINE:  doDefine(buf);
            BCASE INLINE:  doDefine(buf); last->f=IS_INLINE;
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

void initialWords() {
    // VM Opcodes
    char *m1i = ":I %s :M #%ld 3";       // MACHINE-INLINE/one
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
    parseF(m1i, "ZTYPE", ZTYPE);
    parseF(m1i, "EXIT", EXIT);

    char *m2i = ":I %s :M #%ld #%ld 3";  // MACHINE-INLINE/two
    parseF(m2i, ",",     SYS_OPS, COMMA);
    parseF(m2i, "c,",    SYS_OPS, CCOMMA);
    parseF(m2i, "(.)",   SYS_OPS, DOT);
    parseF(m2i, "EMIT",  SYS_OPS, EMIT);
    parseF(m2i, "CLOCK", SYS_OPS, TIMER);

    char *nci = ":I %s ]] %d c, ;";      // NUM-CCOMMA-INLINE
    parseF(nci, "LIT,",   LIT);
    parseF(nci, "JMP,",   JMP);
    parseF(nci, "JMPZ, ", JMPZ);
    parseF(nci, "JMPNZ,", JMPNZ);

    // VM Information Words
    char *cni = ":I %s ]] #%ld ;";       // CONSTANT-INLINE
    parseF(cni, "cell", CELL_SZ);

    char *cnn = ":D %s ]] #%zu ;";       // CONSTANT-NORMAL
    parseF(cnn, "(here)",   (cell_t)&here);
    parseF(cnn, "(vhere)",  (cell_t)&vhere);
    parseF(cnn, "(last)",   (cell_t)&last);
    parseF(cnn, "state",    (cell_t)&state);
    parseF(cnn, "base",     (cell_t)&base);
    parseF(cnn, "code",     (cell_t)&code[0]);
    parseF(cnn, "code-end", (cell_t)&code[CODE_SZ]);
    parseF(cnn, "vars",     (cell_t)&vars[0]);
    parseF(cnn, "vars-end", (cell_t)&vars[VARS_SZ]);

    // CF specific words
    parseF(m1i, "EDIT",   EDIT);
    parseF(m1i, "LOAD",   BLOAD);
    parseF(m1i, "WORDS",  WORDS);
    parseF(m1i, "FOPEN",  FOPEN);
    parseF(m1i, "FCLOSE", FCLOSE);
    parseF(m1i, "FREAD",  FREAD);
    parseF(m1i, "FWRITE", FWRITE);

    doOuter(":I  NIP ]] SWAP DROP ;"
        " :I  / ]] /MOD NIP ;"
        " :D  bye ]]  999 state ! ;"
    );
}

void loop() {
    char buf[128];
    printString(" ok\r\n");
    state = INTERP;
    if (fgets(buf, 128, stdin) != buf) {
        state = 999;
        return;
    }
    doOuter(buf);
}

int main(int argc, char **argv) {
    vmInit();
    initialWords();
    output_fp = (cell_t)stdout;
    // doEditor(0);
    doOuter(blockRead(0));
    while (state != 999) { loop(); }
    return 1;
}

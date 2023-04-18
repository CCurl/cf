#include "cf.h"

#if _MSC_VER
#include <conio.h>
int key() { return _getch(); }
int qKey() { return _kbhit(); }
#elif LINUX
#include "linux.inc"
#endif

byte user[USER_SZ], vars[VARS_SZ];
byte *pc, *toIn, mode;
CELL sp, rsp, stk[STK_SZ+1];
byte *rstk[STK_SZ+1];
CELL lsp, lstk[STK_SZ+1];
CELL rb, regs[100];
byte *here;
DICT_T *last;

#define TOS     stk[sp]
#define NOS     stk[sp-1]
#define L0      lstk[lsp]
#define L1      lstk[lsp-1]
#define L2      lstk[lsp-2]
#define NEXT    goto Next

void push(CELL x) { stk[++sp] = x; }
CELL pop() { return stk[sp--]; }

void rpush(byte *v) { rstk[++rsp] = v; }
byte *rpop() { return (0 < rsp) ? rstk[rsp--] : 0; }

void printString(const char *s) { printf("%s", s); }
void printChar(char c) { printf("%c", c); }

#define ERR(x) printString(x);

void printStringF(const char* fmt, ...) {
    char buf[64];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    printString(buf);
}

void Color(int fg, int bg) {
    printStringF("%c[%d;%dm", 27, fg, bg ? bg : 40);
}

#ifdef NEEDS_ALIGN
#define WFO(l) (user[l+1]*256 | user[l])        // WordFromOffset
#define WFA(l) ((*(l+1)*256) | (*l))            // WordFromAddress

void WSA(byte* l, ushort v) { *(l+1)=v/256; *l=v%256; }
void WSO(ushort l, ushort v) { user[l+1]=v/256; user[l]=v%256; }

CELL LFO(ushort l) {
    CELL x = user[l++];
    x += (user[l++]<<8);
    x += (user[l++]<<8);
    x += (user[l]<<8);
    return x;
}

void LSO(ushort l, CELL v) {
    user[l++]=(v%0xff); v=v>>8;
    user[l++]=(v%0xff); v=v>>8;
    user[l++]=(v%0xff); v=v>>8;
    user[l] = (v%0xff);
}
#else
#define CF(a)     *(CELL*)(a)               // CellFetch
#define CS(a, v)  *(CELL*)(a) = v           // CellSet
#define BF(a)     *(byte*)(a)               // ByteFetch
#define BS(a, v)  *(byte*)(a) = (byte)v     // ByteSet
#define AFA(a)    *(byte**)(a)              // AddressFromAddress

#endif

void CCOMMA(CELL x) { *(here++)=(byte)(x); }
void COMMA(CELL val) { CS(here, val); here += CELL_SZ; }

byte *doQuote(byte *pc) {
    while ((*pc) && (*pc != '"')) {
        char c = *(pc++);
        if (c == '%') {
            c = *(pc++);
            if (c == 'd') { printStringF("%d", pop()); }
            else if (c == 'x') { printStringF("%x", pop()); }
            else if (c == 'n') { printString("\r\n"); }
            else if (c == 'b') { printChar(' '); }
            else if (c == 'c') { printChar(pop()); }
            else { printChar(c); }
        } else { printChar(c); }
    } ++pc;
    return pc;
}

void run(byte *pc) {
    CELL t1, t2;
Next:
    if (!betw(pc,&user[0],&user[USER_SZ-1])) {
        printString("-oob-"); return;
    }
    switch (*(pc++)) {
    case STOP:                                                         return;
    case LIT1: push(*(pc++));                                           NEXT;
    case LIT4: push(CF(pc)); pc += CELL_SZ;                             NEXT;
    case CALL: if (*(pc+CELL_SZ)!=';') { rpush(pc+CELL_SZ); }        // fall-thru
    case JMP: pc=AFA(pc);                                               NEXT;
    case JMPz: if (pop()==0) { pc=AFA(pc); } else { pc+=CELL_SZ; }      NEXT;
    case JMPn: if (pop()!=0) { pc=AFA(pc); } else { pc+=CELL_SZ; }      NEXT;
    case ' ': while (*pc==' ') { pc++; }                                NEXT;
    case '"': pc=doQuote(pc);                                           NEXT;
    case '%': t1 = NOS; push(t1);                                       NEXT;
    case '#': t1 = TOS; push(t1);                                       NEXT;
    case '$': t1 = TOS; TOS = NOS; NOS = t1;                            NEXT;
    case '\\': sp = (0<sp) ? sp-1: 0;                                   NEXT;
    case '+': t1 = pop(); TOS += t1;                                    NEXT;
    case '-': t1 = pop(); TOS -= t1;                                    NEXT;
    case '*': t1 = pop(); TOS *= t1;                                    NEXT;
    case '/': t1=*(pc++); t2=pop(); if (t1=='/') { TOS /= t2; }
        else if (t1=='M') { t1=pop(); push(t1%t2); push(t1/t2); }
        else if (t1=='%') { TOS %= t2; }                                NEXT;
    case ';': pc=rpop(); if (!pc) { rsp=0; return; };                   NEXT;
    case '=': t1 = pop(); TOS = (TOS == t1) ? 1 : 0;                    NEXT;
    case '<': t1 = pop(); TOS = (TOS < t1) ? 1 : 0;                     NEXT;
    case '>': t1 = pop(); TOS = (TOS > t1) ? 1 : 0;                     NEXT;
    case '.': printStringF("%d ", pop());                               NEXT;
    case 'e': printChar(pop());                                         NEXT;
    case 'c': t1=*(pc++); if (t1=='!') { BS(TOS,NOS); sp-=2; }
        else if (t1=='@') { TOS=BF(TOS); }
        else if (t1==',') { CCOMMA(pop()); }                            NEXT;
    case 'l': t1=*(pc++); if (t1=='!') { CS(TOS,NOS); sp-=2; }
        else if (t1=='@') { TOS=CF(TOS); }
        else if (t1==',') { COMMA(pop()); }                             NEXT;
    case 'k': t1=*(pc++); if (t1=='?') { push(qKey()); }
        else if (t1=='@') { push(key()); }                              NEXT;
    case 'd': t1=*(pc++)-'0'; betw(t1,0,9) ? --regs[t1+rb] : --TOS;     NEXT;
    case 'i': t1=*(pc++)-'0'; betw(t1,0,9) ? ++regs[t1+rb] : ++TOS;     NEXT;
    case 'r': t1=*(pc++)-'0'; if (betw(t1,0,9)) { push(regs[rb+t1]); }  NEXT;
    case 's': t1=*(pc++)-'0'; if (betw(t1,0,9)) { regs[rb+t1]=pop(); }  NEXT;
    case 't': push(clock());                                            NEXT;
    case 'u': t1=*(pc++); if (t1=='1') { rpush((byte*)pop()); }
            else if (t1=='2') { push((CELL)rstk[rsp]); }
            else if (t1=='3') { push((CELL)rpop()); }                     
            else if (t1=='+') { rb+=(rb< 81) ? 10 : 0; }
            else if (t1=='-') { rb-=(9 < rb) ? 10 : 0; }                NEXT;
    case '[': lsp+=3; L0=pop(); L1=pop(); L2=(CELL)pc;                  NEXT;
    case ']': if (++L0 < L1) { pc=(byte*)L2; } else { lsp-=3; }         NEXT;
    case '{': lsp+=2; L0=pop()-1; L1=(CELL)pc;                          NEXT;
    case '}': if (0 <= --L0) { pc=(byte*)L1; } else { lsp-=2; }         NEXT;
    case 'I': push(L0);                                                 NEXT;
    default: ERR("-ir-");                                              return;
    }
}

char isWS(char c) { return betw(c,1,32) ? 1 : 0; }
char peekChar() { return *toIn; }
char nextChar() { return *(toIn) ? *(toIn++) : 0; }

int getWord(char* buf) {
    char* b = buf;
    int l = 0;
    while (isWS(peekChar())) { nextChar(); }
    while (' ' < peekChar()) {
        *(b++) = nextChar();
        l++;
    }
    *b = 0;
    return l;
}

int isNum(const char* wd) {
    CELL x = 0;
    int base = 10, isNeg = 0, lastCh = '9';
    if ((wd[0]=='#') && (wd[1]) && (!wd[2])) { push(wd[1]); return 1; }
    if (*wd == '$') { base = 16;  ++wd; }
    if (*wd == '%') { base = 2;  ++wd; lastCh = '1'; }
    if ((*wd == '-') && (base == 10)) { isNeg = 1;  ++wd; }
    if (*wd == 0) { return 0; }
    while (*wd) {
        char c = *(wd++);
        int t = -1;
        if (betw(c, '0', lastCh)) { t = c - '0'; }
        if ((base == 16) && (betw(c, 'A', 'F'))) { t = c - 'A' + 10; }
        if ((base == 16) && (betw(c, 'a', 'a'))) { t = c - 'a' + 10; }
        if (t < 0) { return 0; }
        x = (x*base) + t;
    }
    if (isNeg) { x = -x; }
    push(x);
    return 1;
}

void doAsm(const char* wd) {
    while (*wd) { CCOMMA(*(wd++)); }
}

void doDefine(const char* wd, byte fl) {
    // printStringF("-def:%s-", wd);
    --last;
    last->l = (byte)strlen(wd);
    last->xt = here;
    last->fl = fl;
    strcpy(last->name, wd);
}

// find (cp--[dp 1]|0)
void doFind(const char* wd) {
    // printStringF("-find:%s-", wd);
    push(0);
    int l = strlen(wd);
    DICT_T *dp = last;
    while (dp < (DICT_T*)&user[USER_SZ]) {
        if ((l==dp->l) && (_stricmp(dp->name, wd) == 0)) {
            TOS = (CELL)dp;
            push(1);
            break;
        }
        ++dp;
    }
}

void doCompile(const char* wd) {
    // printStringF("-com:%s-", wd);
    if (isNum(wd)) {
        CELL x = pop();
        if (betw(x,0,127)) {
            CCOMMA(LIT1);
            CCOMMA(x);
        } else {
            CCOMMA(LIT4);
            COMMA(x);
        }
        return;
    }
    doFind(wd);
    if (pop()) {
        DICT_T* dp = (DICT_T*)pop();
        if (dp->fl & IMMEDIATE) {
            run(dp->xt);
        } else if (dp->fl & INLINE) {
            byte *x = dp->xt;
            CCOMMA(*(x++));
            while (*x && (*x != ';')) { CCOMMA(*(x++)); }
        } else {
            CCOMMA(CALL);
            COMMA((CELL)dp->xt);
        }
        return;
    }
    while (*wd) { CCOMMA(*(wd++)); }
}

void doInterpret(const char* wd) {
    // printStringF("-interp:%s-", wd);
    if (isNum(wd)) { return; }
    doFind(wd);
    if (pop()) {
        DICT_T* dp = (DICT_T*)pop();
        run(dp->xt);
        return;
    }
    if (strcmp(wd,"edit")==0) {
        doEditor(pop());
        doOuter(theBlock);
        return;
    }
    byte *cp = here;
    while (*wd) { *(cp++) = *(wd++); }
    *cp = 0;
    run(here);
}

int setMode(char c) {
    if (c==COMMENT) { mode=c; return 1; }
    if (c==COMPILE) { mode=c; return 1; }
    if (c==DEFINE) { mode=c; return 1; }
    if (c==IMMED) { mode=c; return 1; }
    if (c==ASM) { mode=c; return 1; }
    return 0;
}

void doOuter(char* cp) {
    char buf[32];
    toIn = cp;
    while (1) {
        while (isWS(peekChar())) { nextChar(); }
        char c = peekChar();
        if (!c) { return; }
        if (setMode(c)) { nextChar(); }
        if (getWord(buf) == 0) { return; }
        switch (mode) {
        case COMMENT:                                   break;
        case DEFINE:  doDefine(buf, 0);                 break;
        case COMPILE: doCompile(buf);                   break;
        case ASM:     doAsm(buf);                       break;
        case IMMED:  doInterpret(buf);                 break;
        default: break;
        }
    }
}

char *rTrim(char *cp) {
    char *p = cp;
    while (*p) { p++; }
    while ((*p < 32) && (cp <= p)) { *(p--) = 0; }
    return cp;
}

void defNum(char *name, CELL val, byte fl) {
    doDefine(name, fl);
    if (betw(val,0,127)) { CCOMMA(LIT1); CCOMMA(val); }
    else { CCOMMA(LIT4); COMMA(val); }
    CCOMMA(';');
}

struct { char *nm; char *code; } ops[] = {
    {"EXIT", ";"}, {"TIMER", "t"},
    {"DUP", "#"},  {"SWAP", "$"},  {"OVER", "%"},  {"DROP", "\\"},
    {"DO", "[" },  {"LOOP", "]"},  {"I", "I"},
    {"FOR", "{" }, {"NEXT", "}"},
    {"KEY", "k@"}, {"?KEY", "k?"},
    {"EMIT", "e"}, {".", "."},
    {">r", "u1"},  {"r@", "u2"},   {"r>", "u3"},
    {"/", "//"},   {"MOD", "/%"},  {"/MOD", "/M"},
    {"!", "l!"},   {"@", "l@"},    {",", "l,"},
    {"1+", "i+"},  {"1-", "d-"},
    {"r+", "u+"},  {"r-", "u-"},
    {0, 0}
};

void initVM() {
    last = (DICT_T*)&user[USER_SZ];
    here = &user[0];
    sp = rsp = lsp = rb = 0;
    mode = IMMED;
    for (int i = 0; ops[i].code; i++) {
        doDefine(ops[i].nm, INLINE);
        for (int j=0; ops[i].code[j]; j++) { CCOMMA(ops[i].code[j]); }
        CCOMMA(';');
    }
    defNum("cell",CELL_SZ, INLINE);
    defNum("(here)",(CELL)&here, 0);
    defNum("(last)",(CELL)&last, 0);
    defNum("user",(CELL)&user[0], 0);
    defNum("user-sz",USER_SZ, 0);
    defNum("vars",(CELL)&vars[0], 0);
    defNum("vars-sz",VARS_SZ, 0);
    doOuter(": here ^(here) @ ;");
    doOuter(": last ^(last) @ ;");
    doOuter(": if ^3 c, here 0 , ;");
    doOuter(": then ^here swap ! ;");
    doOuter(": begin ^here ;");
    doOuter(": again ^2 c, , ;");
    doOuter(": until ^3 c, , ;");
    doOuter(": while ^4 c, , ;");
}

void cfColor(int md) {
    if (md == IMMED) { Color(YELLOW,0); }
    else if (md == DEFINE) { Color(RED,0); }
    else if (md == COMPILE) { Color(CYAN,0); }
    else if (md == COMMENT) { Color(WHITE,0); }
    else if (md == INPUT) { Color(GREEN,0); }
}

int loop() {
    char buf[96];
    sp = (sp<1) ? 0 : sp;
    lsp = (lsp<1) ? 0 : lsp;
    mode = IMMED;
    cfColor(mode);
    printf(" ok\r\n");
    cfColor(INPUT);
    fgets(buf, 96, stdin);
    cfColor(mode);
    if (strcmp(rTrim(buf), "edit") == 0) {
        if (sp==0) { push(0); }
        doEditor(pop());
        initVM();
        doOuter(theBlock);
    } else if (strcmp(buf, "bye") == 0) {
        return 0;
    } else {
        doOuter(buf);
    }
    return 1;
}

int main(int argc, char **argv) {
    initVM();
    while (loop()) {}
    return 1;
}

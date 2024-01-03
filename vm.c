// vm - a stack-based VM

#include "vm.h"

stk_t ds, rs;
cell_t lstk[LSTK_SZ+1], lsp;
cell_t state, base, reg[REGS_SZ], reg_base, t1, n1;
char code[CODE_SZ], vars[VARS_SZ], tib[256], WD[32];
char *here, *vhere, *in, *y;
dict_t tempWords[10], *last;

void push(cell_t x) { ds.stk[++DSP].i = (cell_t)(x); }
cell_t pop() { return ds.stk[DSP--].i; }
char *cpop() { return ds.stk[DSP--].c; }

void fpush(flt_t x) { ds.stk[++DSP].f = (x); }
flt_t fpop() { return ds.stk[DSP--].f; }

void rpush(char *x) { rs.stk[++RSP].c = (x); }
char *rpop() { return rs.stk[RSP--].c; }

void CComma(cell_t x) { *(here++) = (char)x; }
void Comma(cell_t x) { Store(here, x); here += CELL_SZ; }

void fill(char *d, char val, int num) { for (int i=0; i<num; i++) { d[i]=val; } }
char *strEnd(char *s) { while (*s) { ++s; } return s; }
void strCat(char *d, const char *s) { d=strEnd(d); while (*s) { *(d++)=*(s++); } *d=0; }
void strCatC(char *d, const char c) { d=strEnd(d); *(d++)=c; *d=0; }
void strCpy(char *d, const char *s) { if (d != s) { *d = 0; strCat(d, s); } }
int  strLen(const char *d) { int len = 0; while (*d++) { ++len; } return len; }
char *lTrim(char *d) { while (*d && (*d<33)) { ++d; } return d; }
char *rTrim(char *d) { char *s=strEnd(d)-1; while ((d<=s) && (*s< 33)) { *(s--) = 0; } return d; }
int lower(int x) { return BTW(x,'A','Z') ? x+32: x; }
int upper(int x) { return BTW(x,'a','z') ? x-32: x; }
int min(int a, int b) { return (a < b) ? a : b; }
int max(int a, int b) { return (a > b) ? a : b; }

int strEqI(const char *s, const char *d) {
    while (lower(*s++) == lower(*d++)) { if (*(s-1)==0) { return 1; } }
    return 0;
}

int strEq(const char *d, const char *s) {
    while (*(s++) == *(d++)) { if (*(s-1)==0) { return 1; } }
    return 0;
}

void printStringF(const char *fmt, ...) {
    char *buf = (char*)last;
    buf -= 256;
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, 200, fmt, args);
    va_end(args);
    printString(buf);
}

char *iToA(cell_t N, int b) {
    static char buf[65];
    ucell_t X = (ucell_t)N;
    int isNeg = 0;
    if (b == 0) { b = (int)base; }
    if ((b == 10) && (N < 0)) { isNeg = 1; X = -N; }
    char c, *cp = &buf[64];
    *(cp) = 0;
    do {
        c = (char)(X % b) + '0';
        X /= b;
        c = (c > '9') ? c+7 : c;
        *(--cp) = c;
    } while (X);
    if (isNeg) { *(--cp) = '-'; }
    return cp;
}

int isTempWord(const char *w) {
    return ((w[0]=='T') && BTW(w[1],'0','9') && (w[2]==0)) ? 1 : 0;
}

char isRegOp(const char *w) {
    if (!BTW(w[1],'0','9')) { return 0; }
    if (w[0]=='r') {
        if (w[2]==0) { return REG_R; }
        if ((w[2]=='-') && (w[3]==0))  { return REG_RD; }
        if ((w[2]=='+') && (w[3]==0))  { return REG_RI; }
        return 0;
    }
    if (w[2]) { return 0; }
    if (w[0]=='s') { return REG_S; }
    if (w[0]=='i') { return REG_I; }
    if (w[0]=='d') { return REG_D; }
    return 0;
}

int nextWord(char *wd) {
    int len = 0;
    if (DSP < 0) { printString("-under-"); DSP=0; }
    if (STK_SZ < DSP) { printString("-over-"); DSP=STK_SZ; }
    while (*in && (*in < 33)) { ++in; }
    while (32 < *in) { wd[len++] = *(in++); }
    wd[len] = 0;
    return len;
}

void doDefine(char *wd) {
    if (!wd) { nextWord(WD); wd = WD; }
    // printf("\n def [%s] at %p ... ", wd, here);
    if (isTempWord(wd)) { tempWords[wd[1]-'0'].xt = (cell_t)here; return; }
    int l = strLen(wd);
    --last;
    if (NAME_LEN < l) { l=NAME_LEN; wd[l]=0; printString("-nameTrunc-"); }
    strCpy(last->name, wd);
    last->len = l;
    last->xt = (cell_t)here;
    last->f = 0;
}

// ( nm--xt flags | <null> )
int doFind(const char *nm) {
    if (nm == 0) { nextWord(WD); nm = WD; }
    if (isTempWord(nm)) {
        n1 = nm[1]-'0';
        push(tempWords[n1].xt);
        push(tempWords[n1].f);
        return 1;
    }
    int len = strLen(nm);
    dict_t *dp = last;
    while (dp < (dict_t*)&code[CODE_SZ]) {
        if ((len==dp->len) && strEqI(nm, dp->name)) {
            push(dp->xt);
            push(dp->f);
            return 1;
        }
        ++dp;
    }
    return 0;
}

// ( --n | <null> )
int isBase10(const char *wd) {
    cell_t x = 0, isNeg = (*wd == '-') ? 1 : 0;
    if (isNeg && (*(++wd) == 0)) { return 0; }
    if (!BTW(*wd, '0', '9')) { return 0; }
    while (BTW(*wd, '0', '9')) { x = (x*10)+(*(wd++)-'0'); }
    if (*wd == 0) { push(isNeg ? -x : x); return 1; }
    if (*(wd++) != '.') { return 0; }
    flt_t fx = (flt_t)x, f = 0, fy = 1;
    while (BTW(*wd, '0', '9')) { f = (f*10)+(*(wd++)-'0'); fy*=10; }
    if (*wd) { return 0; }
    fx += f/fy;
    fpush(isNeg ? -fx : fx);
    return 1;
}

// ( --n | <null> )
int isNum(const char *wd) {
    if ((wd[0]=='\'') && (wd[2]=='\'') && (wd[3]==0)) { push(wd[1]); return 1; }
    int b = (int)base, lastCh = '9';
    if (*wd == '#') { b = 10;  ++wd; }
    if (*wd == '$') { b = 16;  ++wd; }
    if (*wd == '%') { b =  2;  ++wd; }
    if (*wd == 0) { return 0; }
    if (b == 10) { return isBase10(wd); }
    if (b < 10) { lastCh = '0' + b - 1; }
    cell_t x = 0;
    while (*wd) {
        cell_t t = -1, c = *(wd++);
        if (BTW(c, '0', lastCh)) { t = c - '0'; }
        if ((b == 16) && (BTW(c, 'A', 'F'))) { t = c - 'A' + 10; }
        if ((b == 16) && (BTW(c, 'a', 'f'))) { t = c - 'a' + 10; }
        if (t < 0) { return 0; }
        x = (x * b) + t;
    }
    push(x);
    return 1;
}

void doType(const char *str) {
    if (!str) { str=cpop(); }
    for (int i = 0; i < str[i]; i++) {
        char c = str[i];
        if (c == '%') {
            c = str[++i];
            if (c == 0) { return; }
            else if (c=='b') { printString(iToA(pop(), 2)); }
            else if (c=='c') { printChar((char)pop()); }
            else if (c=='d') { printString(iToA(pop(), 10)); }
            else if (c=='e') { printChar(27); }
            else if (c=='f') { printStringF("%f", fpop()); }
            else if (c=='g') { printStringF("%g", fpop()); }
            else if (c=='i') { printString(iToA(pop(), (int)base)); }
            else if (c=='n') { printString("\n\r"); }
            else if (c=='q') { printChar(34); }
            else if (c=='s') { printString(cpop()); }
            else if (c=='t') { printChar(9); }
            else if (c=='x') { printString(iToA(pop(), 16)); }
            else { printChar(c); }
        } else {
            printChar(c);
        }
    }
}

char *doStringOp(char *pc) {
    char *d, *s;
    switch(*pc++) {
        case TRUNC:    d=cpop(); d[0] = 0;
        RCASE LCASE:   TOS = lower((int)TOS);
        RCASE UCASE:   TOS = upper((int)TOS);
        RCASE STRCPY:  s=cpop(); d=cpop(); strCpy(d, s);
        RCASE STRCAT:  s=cpop(); d=cpop(); strCat(d, s);
        RCASE STRCATC: t1=pop(); d=cpop(); strCatC(d, (char)t1);
        RCASE STRLEN:  TOS=strLen(CTOS);
        RCASE STREQ:   s=cpop(); d=CTOS; TOS=strEq(d, s);
        RCASE STREQI:  s=cpop(); d=CTOS; TOS=strEqI(d, s);
        RCASE LTRIM:   CTOS=lTrim(CTOS);
        RCASE RTRIM:   rTrim(CTOS);
            return pc;
        default: printStringF("-strOp:[%d]?-", *(pc-1));
    }
    return pc;
}

char *doSysOp(char *pc) {
    switch(*pc++) {
        case INLINE: last->f = IS_INLINE;
        RCASE IMMEDIATE: last->f = IS_IMMEDIATE;
        RCASE DOT: printString(iToA(pop(), (int)base));
        RCASE ITOA: TOS = (cell_t)iToA(TOS, (int)base);
        RCASE ATOI: push(isNum(cpop()));
        RCASE COLONDEF: doDefine(NULL); state=1;
        RCASE ENDWORD: state=0; CComma(EXIT);
        RCASE CREATE: doDefine(NULL); CComma(LIT); Comma((cell_t)vhere);
        RCASE FIND: push(doFind(ToCP(0)));
        RCASE WORD: t1=nextWord(WD); push((cell_t)WD); push(t1);
        RCASE TIMER: push(sysTime());
        RCASE CCOMMA: CComma(pop());
        RCASE COMMA: Comma(pop());
        RCASE KEY:  push(key());
        RCASE QKEY: push(qKey());
        RCASE EMIT: printChar((char)pop());
        RCASE QTYPE: printString(cpop());
            return pc;
        default: printStringF("-sysOp:[%d]?-", *(pc-1));
    }
    return pc;
}

char *doFloatOp(char *pc) {
    flt_t x;
    switch(*pc++) {
        case  FADD: x = fpop(); FTOS += x;
        RCASE FSUB: x = fpop(); FTOS -= x;
        RCASE FMUL: x = fpop(); FTOS *= x;
        RCASE FDIV: x = fpop(); FTOS /= x;
        RCASE FEQ:  x = fpop(); TOS = (x == FTOS);
        RCASE FLT:  x = fpop(); TOS = (x > FTOS);
        RCASE FGT:  x = fpop(); TOS = (x < FTOS);
        RCASE F2I:  TOS = (cell_t)FTOS;
        RCASE I2F:  FTOS = (flt_t)TOS;
        RCASE FDOT: printStringF("%g", fpop());
        RCASE SQRT: FTOS = (flt_t)sqrt(FTOS);
        RCASE TANH: FTOS = (flt_t)tanh(FTOS);
            return pc;
        default: printStringF("-fltOp:[%d]?-", *(pc-1));
    }
    return pc;
}

void Run(char *pc) {
    flt_t f1;
next:
    switch (*(pc++)) {
        case STOP: return;
        NCASE LIT1: push(*(pc++));
        NCASE LIT: push(Fetch(pc)); pc += CELL_SZ;
        NCASE EXIT: if (RSP<1) { RSP=0; return; } pc=rpop();
        NCASE CALL: y=pc+CELL_SZ; if (*y!=EXIT) { rpush(y); }          // fall-thru
        case  JMP: pc = CpAt(pc);
        NCASE JMPZ:  if (pop()==0) { pc=CpAt(pc); } else { pc+=CELL_SZ; }
        NCASE JMPNZ: if (TOS) { pc=CpAt(pc); } else { pc+=CELL_SZ; }
        NCASE STORE:  Store(CTOS, NOS);  DSP-=2; if (DSP < 1) { DSP = 0; }
        NCASE CSTORE: *CTOS = (char)NOS; DSP-=2; if (DSP < 1) { DSP = 0; }
        NCASE FETCH: TOS = Fetch(CTOS);
        NCASE CFETCH: TOS = *(byte*)(TOS);
        NCASE DUP: fpush(FTOS);
        NCASE SWAP: f1 = FTOS; FTOS = FNOS; FNOS = f1;
        NCASE OVER: fpush(FNOS);
        NCASE DROP: if (--DSP < 0) { DSP = 0; }
        NCASE ADD:   t1=pop(); TOS += t1;
        NCASE MULT:  t1=pop(); TOS *= t1;
        NCASE SLMOD: t1=TOS; TOS = (NOS/t1); NOS %= t1;
        NCASE SUB:   t1=pop(); TOS -= t1;
        NCASE INC: ++TOS;
        NCASE DEC: --TOS;
        NCASE LT: t1=pop(); TOS = (TOS<t1);
        NCASE EQ: t1=pop(); TOS = (TOS==t1);
        NCASE GT: t1=pop(); TOS = (TOS>t1);
        NCASE EQ0: TOS = (TOS==0);
        NCASE RTO: rpush((char *)pop());
        NCASE RFETCH: push((cell_t)RTOS);
        NCASE RFROM: push((cell_t)rpop());
        NCASE DO: lsp+=3; L2=(cell_t)pc; L0=pop(); L1=pop();
        NCASE LOOP:  if (++L0<L1) { pc=ToCP(L2); } else { lsp-=3; };
        NCASE LOOP2: if (--L0>L1) { pc=ToCP(L2); } else { lsp-=3; };
        NCASE INDEX: push((cell_t)&L0);
        NCASE COM: TOS = ~TOS;
        NCASE AND: t1=pop(); TOS = (TOS & t1);
        NCASE OR:  t1=pop(); TOS = (TOS | t1);
        NCASE XOR: t1=pop(); TOS = (TOS ^ t1);
        NCASE TYPE: t1=pop(); y=cpop(); for (int i=0; i<t1; i++) { printChar(*(y++)); }
        NCASE ZTYPE: doType(0);
        NCASE REG_I: reg[*(pc++)+reg_base]++;
        NCASE REG_D: reg[*(pc++)+reg_base]--;
        NCASE REG_R:  push(reg[*(pc++)+reg_base]);
        NCASE REG_RD: push(reg[*(pc++)+reg_base]--);
        NCASE REG_RI: push(reg[*(pc++)+reg_base]++);
        NCASE REG_S: reg[*(pc++)+reg_base] = pop();
        NCASE REG_NEW: reg_base += (reg_base < (REGS_SZ-10)) ? 10 : 0;
        NCASE REG_FREE: reg_base -= (9 < reg_base) ? 10 : 0;
        NCASE SYS_OPS: pc = doSysOp(pc);
        NCASE STR_OPS: pc = doStringOp(pc);
        NCASE FLT_OPS: pc = doFloatOp(pc);
            goto next;
        default: pc = doUser(pc, *(pc-1));
            if (pc) { goto next; }
            printStringF("-op:[%d]?-", *(pc-1));
    }
}

int doReg(const char *w) {
    char t = isRegOp(w);
    if (t == 0) { return 0; }
    if (state) { CComma(t); CComma(w[1]-'0'); }
    else {
        int h=245;
        tib[h]=t; tib[h+1]=w[1]-'0'; tib[h+2]=EXIT;
        Run(&tib[h]);
    }
    return 1;
}

void vmInit() {
    here = &code[0];
    vhere = &vars[0];
    last = (dict_t*)&code[CODE_SZ];
    base = 10;
    DSP = RSP = reg_base = 0;

    for (int i=0; i<6; i++) { tempWords[i].f = 0; }
    for (int i=6; i<9; i++) { tempWords[i].f = IS_INLINE; }
    tempWords[9].f = IS_IMMEDIATE;
}

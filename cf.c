// A ColorForth and Tachyon inspired system, MIT license.

#include "cf.h"

#define TOS           dstk[dsp]
#define NOS           dstk[dsp-1]
#define L0            lstk[lsp]
#define L1            lstk[lsp-1]
#define L2            lstk[lsp-2]
#define isTemp(w)     ((w[0]=='t') && btwi(w[1],'0','9') && (w[2]==0))
#define X1(op, name, code) op,
#define X2(op, name, code) NCASE op: code
#define X3(op, name, code) { op, name, 0 },

byte mem[MEM_SZ];
cell dsp, dstk[STK_SZ+1],  rsp, rstk[STK_SZ+1],  lsp, lstk[LSTK_SZ+1];
cell asp, astk[TSTK_SZ+1], bsp, bstk[TSTK_SZ+1], tsp, tstk[TSTK_SZ+1];
cell base, state, dictEnd, outputFp;
cell *code, here, vhere, last;
char *toIn, wd[128];
DE_T tmpWords[10];

#define PRIMS(X) \
	X(DUP,     "dup",     t=TOS; push(t); ) \
	X(SWAP,    "swap",    t=TOS; TOS=NOS; NOS=t; ) \
	X(DROP,    "drop",    pop(); ) \
	X(OVER,    "over",    t=NOS; push(t); ) \
	X(FET,     "@",       TOS = *(cell*)TOS; ) \
	X(STO,     "!",       t=pop(); n=pop(); *(cell*)t = n; ) \
	X(CFET,    "c@",      TOS = *(byte*)TOS; ) \
	X(CSTO,    "c!",      t=pop(); n=pop(); *(byte*)t = (byte)n; ) \
	X(PLSTO,   "+!",      t=pop(); n=pop(); *(cell*)t += n; ) \
	X(ADD,     "+",       t=pop(); TOS += t; ) \
	X(SUB,     "-",       t=pop(); TOS -= t; ) \
	X(MUL,     "*",       t=pop(); TOS *= t; ) \
	X(DIV,     "/",       t=pop(); TOS /= t; ) \
	X(SLMOD,   "/mod",    t=TOS; n = NOS; TOS = n/t; NOS = n%t; ) \
	X(INCR,    "1+",      ++TOS; ) \
	X(DECR,    "1-",      --TOS; ) \
	X(LT,      "<",       t=pop(); TOS = (TOS < t); ) \
	X(EQ,      "=",       t=pop(); TOS = (TOS == t); ) \
	X(GT,      ">",       t=pop(); TOS = (TOS > t); ) \
	X(EXIT,    "exit",    if (0<rsp) { pc = rpop(); } else { return; } ) \
	X(EQ0,     "0=",      TOS = (TOS == 0) ? 1 : 0; ) \
	X(AND,     "and",     t=pop(); TOS &= t; ) \
	X(OR,      "or",      t=pop(); TOS |= t; ) \
	X(XOR,     "xor",     t=pop(); TOS ^= t; ) \
	X(COM,     "com",     TOS = ~TOS; ) \
	X(FOR,     "for",     lsp+=3; L0=0; L1=pop(); L2=pc; ) \
	X(NDX_I,   "i",       push(L0); ) \
	X(LITC,    "lit,",    compileNumber(pop()); ) \
	X(NEXT,    "next",    if (++L0<L1) { pc=L2; } else { lsp=(2<lsp)?(lsp-3):0; } ) \
	X(TOR,     ">r",      rpush(pop()); ) \
	X(RAT,     "r@",      push(rstk[rsp]); ) \
	X(RFROM,   "r>",      push(rpop()); ) \
	X(RDROP,   "rdrop",   rpop(); ) \
	X(ATO,     ">a",      if (asp < STK_SZ) { astk[++asp] = pop(); } ) \
	X(ASET,    "a!",      astk[asp] = pop(); ) \
	X(AGET,    "a@",      push(astk[asp]); ) \
	X(AGETI,   "a@+",     push(astk[asp]++); ) \
	X(AGETD,   "a@-",     push(astk[asp]--); ) \
	X(AFROM,   "a>",      push(0<asp ? astk[asp--]: 0); ) \
	X(BTO,     ">b",      if (bsp < STK_SZ) { bstk[++bsp] = pop(); } ) \
	X(BSET,    "b!",      bstk[bsp] = pop(); ) \
	X(BGET,    "b@",      push(bstk[bsp]); ) \
	X(BGETI,   "b@+",     push(bstk[bsp]++); ) \
	X(BGETD,   "b@-",     push(bstk[bsp]--); ) \
	X(BFROM,   "b>",      push(0<bsp ? bstk[bsp--]: 0); ) \
	X(TTO,     ">t",      if (tsp < STK_SZ) { tstk[++tsp] = pop(); } ) \
	X(TSET,    "t!",      tstk[tsp] = pop(); ) \
	X(TGET,    "t@",      push(tstk[tsp]); ) \
	X(TGETI,   "t@+",     push(tstk[tsp]++); ) \
	X(TGETD,   "t@-",     push(tstk[tsp]--); ) \
	X(TFROM,   "t>",      push(0<tsp ? tstk[tsp--]: 0); ) \
	X(EMIT,    "emit",    emit((char)pop()); ) \
	X(KEY,     "key",     push(key()); ) \
	X(KEYQ,    "key?",    push(qKey()); ) \
	X(OUTER,   "outer",   t=pop(); n=(cell)toIn; cfOuter((char*)t); toIn=(char*)n; ) \
	X(ADDWORD, "addword", addWord(0); ) \
	X(NXTWORD, "nextword",nextWord(); push((cell)wd); ) \
	X(FIND,    "find",    { DE_T *dp=findWord(0); push(dp?dp->xt:0); push((cell)dp); } ) \
	X(CLK,     "timer",   push(timer()); ) \
	X(MS,      "ms",      ms(pop()); ) \
	X(ZTYPE,   "ztype",   zType((const char *)pop()); ) \
	X(FOPEN,   "fopen",   t=pop(); TOS=fOpen(TOS, t); ) \
	X(FCLOSE,  "fclose",  t=pop(); fClose(t); ) \
	X(FREAD,   "fread",   t=pop(); n=pop(); TOS=fRead(TOS, n, t); ) \
	X(FWRITE,  "fwrite",  t=pop(); n=pop(); TOS=fWrite(TOS, n, t); ) \
	X(FSEEK,   "fseek",   t=pop(); n=pop(); fSeek(t,n); ) \
	X(SYSTEM,  "system",  t=pop(); ttyMode(0); system((char*)t); ) \
	X(SCOPY,   "s-cpy",   t=pop(); strcpy((char*)TOS, (char*)t); ) \
	X(SEQI,    "s-eqi",   t=pop(); n=pop(); push(strEqI((char*)n, (char*)t)); ) \
	X(SLEN,    "s-len",   TOS=strlen((char*)TOS); ) \
	X(CMOVE,   "cmove",   t=pop(); n=pop(); memmove((char*)n, (char*)pop(), t); ) \
	X(BYE,     "bye",     ttyMode(0); exit(0); )

enum { STOP, LIT, JMP, JMPZ, NJMPZ, JMPNZ, NJMPNZ, PRIMS(X1) };

static void push(cell x) { if (dsp < STK_SZ) { dstk[++dsp] = x; } }
static cell pop() { return (0<dsp) ? dstk[dsp--] : 0; }
static void rpush(cell x) { if (rsp < STK_SZ) { rstk[++rsp] = x; } }
static cell rpop() { return (0<rsp) ? rstk[rsp--] : 0; }
static void comma(cell n) { code[here++] = n; }
static int  changeState(int newState) { state = newState; return newState; }
static void checkWS(char c) { if (btwi(c,DEFINE,COMMENT)) { changeState(c); } }

static int nextWord() {
	int len = 0;
	while (btwi(*toIn, 1, 32)) { checkWS(*(toIn++)); }
	while (btwi(*toIn, 33, 126)) { wd[len++] = *(toIn++); }
	wd[len] = 0;
	return len;
}

static DE_T *addWord(char *w) {
	if (!w) { nextWord(); w = wd; }
	if (isTemp(w)) {
		tmpWords[w[1]-'0'].xt = (cell)here;
		return &tmpWords[w[1]-'0'];
	}
	int ln = (int)strlen(w);
	if (ln > NAME_MAX) { ln=NAME_MAX; w[ln]=0; }
	last -= sizeof(DE_T);
	DE_T *dp = (DE_T*)last;
	dp->xt = (cell)here;
	dp->flags = 0;
	dp->len = ln;
	strcpy(dp->name, w);
	return dp;
}

static DE_T *findWord(const char *w) {
	if (!w) { nextWord(); w = wd; }
	if (isTemp(w)) { return &tmpWords[w[1] - '0']; }
	int ln = (int)strlen(w);
	cell cw = last;
	while (cw < dictEnd) {
		DE_T *dp = (DE_T*)cw;
		if ((ln == dp->len) && strEqI(dp->name, w)) { return dp; }
		cw += sizeof(DE_T);
	}
	return (DE_T*)0;
}

static void compileNumber(cell n) {
		if (btwi(n, 0, NUM_MASK)) { comma(n | NUM_BITS); }
		else { comma(LIT); comma(n); }
}

static void cfInner(cell pc) {
	cell t, n, ir;
	next: ir = code[pc++];
	switch(ir) {
		case  STOP:   return;
		NCASE LIT:    push(code[pc++]);
		NCASE JMP:    pc=code[pc];
		NCASE JMPZ:   t=code[pc++]; if (pop()==0) { pc = t; }
		NCASE JMPNZ:  t=code[pc++]; if (pop())    { pc = t; }
		NCASE NJMPZ:  t=code[pc++]; if (TOS==0)   { pc = t; }
		NCASE NJMPNZ: t=code[pc++]; if (TOS)      { pc = t; }
		PRIMS(X2)
		default:
			if ((ir & NUM_BITS) == NUM_BITS) { push(ir & NUM_MASK); goto next; }
			if (code[pc] != EXIT) { rpush(pc); }
			pc = ir;
			goto next;
	}
}

static int isNumber(const char *w) {
	cell n=0, b=base, isNeg=0;
	if ((w[0]==39) && (w[2]==39) && (w[3]==0)) { push(w[1]); return 1; }
	if (w[0]=='#') { b=10; w++; }
	if (w[0]=='$') { b=16; w++; }
	if (w[0]=='%') { b=2; w++; }
	if (w[0]=='-') { isNeg=1; w++; }
	if (w[0]==0) { return 0; }
	while (*w) {
		int a = *(w++), c = btwi(a,'A','Z') ? (a+32) : a;
		int x = btwi(c,'0','9') ? c-'0' : 99;
		if (btwi(c,'a','f')) { x = (c-'a'+10); }
		if (btwi(x, 0, b-1)) { n = (n*b)+x; } else { return 0; }
	}
	push(isNeg ? -n : n);
	return 1;
}

static void executeWord(DE_T *dp) {
	code[17] = dp->xt;
	code[18] = STOP;
	cfInner(17);
}

static void compileWord(DE_T *dp) {
	if (dp->flags & _IMMED) { executeWord(dp); }
	else if ((dp->flags & _INLINE)) {
		ucell x = dp->xt;
		do { comma(code[x++]); } while ( code[x] != EXIT );
	} else { comma(dp->xt); }
}

static int isStateChange() {
	if (state == COMMENT) { return COMMENT; }
	if (strEqI(wd, ":"))  { return changeState(DEFINE); }
	if (strEqI(wd, ";"))  { comma(EXIT); return changeState(INTERP); }
	if (strEqI(wd, "["))  { return changeState(INTERP); }
	if (strEqI(wd, "]"))  { return changeState(COMPILE); }
	if (strEqI(wd, "("))  {
		while (wd[0] && !strEqI(wd, ")")) { nextWord(); }
		return 1;
	}
	return 0;
}

void cfOuter(const char *src) {
	char *svIn = toIn;
	toIn = (char*)src;
	while (nextWord()) {
		if (isStateChange()) { continue; }
		if (state == DEFINE)  { if (addWord(wd)) { state = COMPILE; continue; } }
		if (isNumber(wd)) {
			if (state == COMPILE) { compileNumber(pop()); }
			continue;
		}
		DE_T *dp = findWord(wd);
		if (!dp) {
			zType("-["); zType(wd); zType("]?-");
			state = INTERP;
			break;
		}
		if (state == INTERP) { executeWord(dp); continue; }
		if (state == COMPILE) { compileWord(dp); continue; }
		zType("-state?-"); zType(wd);
		state = INTERP;
		break;
	}
	toIn = svIn;
}

void cfInit() {
	code  = (cell*)&mem[0];
	here  = BYE+1;
	base  = 10;
	state = INTERP;
	last  = (cell)&mem[MEM_SZ-1];
	while (last & (CELL_SZ-1)) { --last; }
	dictEnd = last;
	vhere = dsp = rsp = lsp = tsp = asp = 0;
    struct { char *nm; cell val; } nvp[] = {
		{ "(ztype)", ZTYPE },    { "(jmp)",   JMP },    { "(jmpz)",   JMPZ },
		{ "(jmpnz)", JMPNZ },    { "(njmpz)", NJMPZ },  { "(njmpnz)", NJMPNZ },
		{ "(lit)",   LIT },      { "(exit)",  EXIT },   { "(bye)",    BYE },
		{ "(dsp)",   (cell)&dsp },     { "dstk",  (cell)&dstk[0] },
		{ "(rsp)",   (cell)&rsp },     { "rstk",  (cell)&rstk[0] },
		{ "(tsp)",   (cell)&tsp },     { "tstk",  (cell)&tstk[0] },
		{ "(asp)",   (cell)&asp },     { "astk",  (cell)&astk[0] },
		{ "(lsp)",   (cell)&lsp },     { "lstk",  (cell)&lstk[0] },
		{ "(bsp)",   (cell)&bsp },     { "bstk",  (cell)&bstk[0] },
		{ "(ha)",    (cell)&here },    { "(vha)", (cell)&vhere },
		{ "(la)",    (cell)&last },    { "(output-fp)",(cell)&outputFp },
		{ "de-sz",   sizeof(DE_T) },   { "stk-sz",  STK_SZ+1 },
		{ "tstk-sz", TSTK_SZ+1 },      { "lstk-sz", LSTK_SZ+1 },
		{ "memory",  (cell)&mem[0] },  { ">in",   (cell)&toIn },
		{ "mem-sz",  MEM_SZ },         { "base",  (cell)&base },
		{ "version", VERSION },        { "state", (cell)&state },
		{ "cell",    CELL_SZ },        { "word",  (cell)&wd[0]},
		{ 0 ,0 }
	};
	for (int i = 0; nvp[i].nm; i++) {
		DE_T *dp = addWord(nvp[i].nm);
		compileNumber(nvp[i].val);
		if (btwi(nvp[i].val, 0, NUM_MASK)) { dp->flags = _INLINE; }
		comma(EXIT);
	}

	PRIM_T prims[] = { PRIMS(X3) {0, 0, 0} };
	for (int i = 0; prims[i].name; i++) {
		addWord((char*)prims[i].name)->xt = prims[i].op;
	}
}

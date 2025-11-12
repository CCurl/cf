// A ColorForth inspired system, MIT license

#include "cf.h"

#define TOS           dstk[dsp]
#define NOS           dstk[dsp-1]
#define L0            lstk[lsp]
#define L1            lstk[lsp-1]
#define L2            lstk[lsp-2]
#define isTemp(w)     ((w[0]=='t') && btwi(w[1],'0','9') && (w[2]==0))

byte mem[MEM_SZ];
cell dsp, dstk[STK_SZ+1],  rsp, rstk[STK_SZ+1];
cell asp, astk[TSTK_SZ+1], bsp, bstk[TSTK_SZ+1];
cell tsp, tstk[TSTK_SZ+1], lsp, lstk[LSTK_SZ+1];
cell base, state, dictEnd, outputFp;
cell *code, here, vhere, last;
char *toIn, wd[128];
DE_T tmpWords[10];

#define PRIMS(X) \
	X(DUP,     "dup",     0, t=TOS; push(t); ) \
	X(SWAP,    "swap",    0, t=TOS; TOS=NOS; NOS=t; ) \
	X(DROP,    "drop",    0, pop(); ) \
	X(OVER,    "over",    0, t=NOS; push(t); ) \
	X(FET,     "@",       0, TOS = fetchCell(TOS); ) \
	X(STO,     "!",       0, t=pop(); n=pop(); storeCell(t, n); ) \
	X(CFET,    "c@",      0, TOS = *(byte *)TOS; ) \
	X(CSTO,    "c!",      0, t=pop(); n=pop(); *(byte*)t=(byte)n; ) \
	X(PLSTO,   "+!",      0, t=pop(); n=pop(); storeCell(t, fetchCell(t)+n); ) \
	X(ADD,     "+",       0, t=pop(); TOS += t; ) \
	X(SUB,     "-",       0, t=pop(); TOS -= t; ) \
	X(MUL,     "*",       0, t=pop(); TOS *= t; ) \
	X(DIV,     "/",       0, t=pop(); TOS /= t; ) \
	X(SLMOD,   "/mod",    0, t=TOS; n = NOS; TOS = n/t; NOS = n%t; ) \
	X(INCR,    "1+",      0, ++TOS; ) \
	X(DECR,    "1-",      0, --TOS; ) \
	X(LT,      "<",       0, t=pop(); TOS = (TOS < t); ) \
	X(EQ,      "=",       0, t=pop(); TOS = (TOS == t); ) \
	X(GT,      ">",       0, t=pop(); TOS = (TOS > t); ) \
	X(EXIT,    "exit",    0, if (0<rsp) { pc = rpop(); } else { return; } ) \
	X(EQ0,     "0=",      0, TOS = (TOS == 0) ? 1 : 0; ) \
	X(AND,     "and",     0, t=pop(); TOS &= t; ) \
	X(OR,      "or",      0, t=pop(); TOS |= t; ) \
	X(XOR,     "xor",     0, t=pop(); TOS ^= t; ) \
	X(COM,     "com",     0, TOS = ~TOS; ) \
	X(FOR,     "for",     0, lsp+=3; L0=0; L1=pop(); L2=pc; ) \
	X(NDX_I,   "i",       0, push(L0); ) \
	X(NEXT,    "next",    0, if (++L0<L1) { pc=L2; } else { lsp=(2<lsp)?(lsp-3):0; } ) \
	X(TOR,     ">r",      0, rpush(pop()); ) \
	X(RAT,     "r@",      0, push(rstk[rsp]); ) \
	X(RFROM,   "r>",      0, push(rpop()); ) \
	X(RDROP,   "rdrop",   0, rpop(); ) \
	X(ATO,     ">a",      0, if (asp < STK_SZ) { astk[++asp] = pop(); } ) \
	X(ASET,    "a!",      0, astk[asp] = pop(); ) \
	X(AGET,    "a@",      0, push(astk[asp]); ) \
	X(AGETI,   "a@+",     0, push(astk[asp]++); ) \
	X(AGETD,   "a@-",     0, push(astk[asp]--); ) \
	X(AFROM,   "a>",      0, push(0<asp ? astk[asp--]: 0); ) \
	X(BTO,     ">b",      0, if (bsp < STK_SZ) { bstk[++bsp] = pop(); } ) \
	X(BSET,    "b!",      0, bstk[bsp] = pop(); ) \
	X(BGET,    "b@",      0, push(bstk[bsp]); ) \
	X(BGETI,   "b@+",     0, push(bstk[bsp]++); ) \
	X(BGETD,   "b@-",     0, push(bstk[bsp]--); ) \
	X(BFROM,   "b>",      0, push(0<bsp ? bstk[bsp--]: 0); ) \
	X(TTO,     ">t",      0, if (tsp < STK_SZ) { tstk[++tsp] = pop(); } ) \
	X(TSET,    "t!",      0, tstk[tsp] = pop(); ) \
	X(TGET,    "t@",      0, push(tstk[tsp]); ) \
	X(TGETI,   "t@+",     0, push(tstk[tsp]++); ) \
	X(TGETD,   "t@-",     0, push(tstk[tsp]--); ) \
	X(TFROM,   "t>",      0, push(0<tsp ? tstk[tsp--]: 0); ) \
	X(EMIT,    "emit",    0, emit((char)pop()); ) \
	X(KEY,     "key",     0, push(key()); ) \
	X(QKEY,    "?key",    0, push(qKey()); ) \
	X(OUTER,   "outer",   0, t=pop(); n=(cell)toIn; cfOuter((char*)t); toIn=(char*)n; ) \
	X(ADDWORD, "addword", 0, addWord(0); ) \
	X(FIND,    "find",    0, { DE_T *dp=findWord(0); push(dp?dp->xt:0); push((cell)dp); } ) \
	X(CLK,     "timer",   0, push(timer()); ) \
	X(MS,      "ms",      0, ms(pop()); ) \
	X(ZTYPE,   "ztype",   0, zType((const char *)pop()); ) \
	X(FOPEN,   "fopen",   0, t=pop(); TOS=fOpen(TOS, t); ) \
	X(FCLOSE,  "fclose",  0, t=pop(); fClose(t); ) \
	X(FREAD,   "fread",   0, t=pop(); n=pop(); TOS=fRead(TOS, n, t); ) \
	X(FWRITE,  "fwrite",  0, t=pop(); n=pop(); TOS=fWrite(TOS, n, t); ) \
	X(FSEEK,   "fseek",   0, t=pop(); n=pop(); fSeek(t,n); ) \
	X(SYSTEM,  "system",  0, t=pop(); ttyMode(0); system((char*)t); ) \
	X(SCOPY,   "s-cpy",   0, t=pop(); strCpy((char*)TOS, (char*)t); ) \
	X(SEQI,    "s-eqi",   0, t=pop(); n=pop(); push(strEqI((char*)n, (char*)t)); ) \
	X(SLEN,    "s-len",   0, TOS=strLen((char*)TOS); ) \
	X(CMOVE,   "cmove",   0, t=pop(); n=pop(); cmove(pop(), n, t); ) \
	X(BYE,     "bye",     0, ttyMode(0); exit(0); )

#define E1(op, name, imm, code) op,
enum _PRIM  { STOP, LIT, JMP, JMPZ, NJMPZ, JMPNZ, NJMPNZ, PRIMS(E1) };

static void push(cell x) { if (dsp < STK_SZ) { dstk[++dsp] = x; } }
static cell pop() { return (0<dsp) ? dstk[dsp--] : 0; }
static void rpush(cell x) { if (rsp < STK_SZ) { rstk[++rsp] = x; } }
static cell rpop() { return (0<rsp) ? rstk[rsp--] : 0; }
static int  lower(const char c) { return btwi(c, 'A', 'Z') ? c+32 : c; }
static int  strLen(const char *s) { int l = 0; while (s[l]) { l++; } return l; }
static void storeWord(byte *a, cell v) { *(ushort*)(a) = (ushort)v; }
static ushort fetchWord(byte *a) { return *(ushort*)(a); }
static void storeCell(cell a, cell v) { *(cell*)(a) = v; }
static cell fetchCell(cell a) { return *(cell*)(a); }
static void comma(cell n) { code[here++] = n; }
static int  changeState(int newState) { state = newState; return newState; }
static void checkWS(char c) { if (btwi(c,DEFINE,COMMENT)) { changeState(c); } }

static int strEqI(const char *s, const char *d) {
	while (lower(*s) == lower(*d)) { if (*s == 0) { return 1; } s++; d++; }
	return 0;
}

static void strCpy(char *d, const char *s) {
	while (*s) { *(d++) = *(s++); }
	*(d) = 0;
}

static void cmove(cell src, cell dst, cell sz) {
	if ((src == dst) || (sz < 1)) { return; }
	byte *s = (byte*)src, *d = (byte*)dst;
	if (s < d) {
		d += sz-1; s += sz-1;
		while (sz--) { *(d--) = *(s--); }
	} else {
		while (sz--) { *(d++) = *(s++); }
	}
}

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
	int ln = strLen(w);
	if (ln > NAME_MAX) { ln=NAME_MAX; w[ln]=0; }
	last -= sizeof(DE_T);
	DE_T *dp = (DE_T*)last;
	dp->xt = (cell)here;
	dp->flags = 0;
	dp->len = ln;
	strCpy(dp->name, w);
	return dp;
}

static DE_T *findWord(const char *w) {
	if (!w) { nextWord(); w = wd; }
	if (isTemp(w)) { return &tmpWords[w[1] - '0']; }
	int len = strLen(w);
	cell cw = last;
	while (cw < dictEnd) {
		DE_T *dp = (DE_T*)cw;
		if ((len == dp->len) && strEqI(dp->name, w)) { return dp; }
		cw += sizeof(DE_T);
	}
	return (DE_T*)0;
}

#define C1(op, name, imm, code) NCASE op: code

void cfInner(cell pc) {
	cell t, n, ir;
	next:
	ir = code[pc++];
	switch(ir) {
		case  STOP:   return;
		NCASE LIT:    push(code[pc++]);
		NCASE JMP:    pc=code[pc];
		NCASE JMPZ:   t=code[pc++]; if (pop()==0) { pc = t; }
		NCASE JMPNZ:  t=code[pc++]; if (pop())    { pc = t; }
		NCASE NJMPZ:  t=code[pc++]; if (TOS==0)   { pc = t; }
		NCASE NJMPNZ: t=code[pc++]; if (TOS)      { pc = t; }
		PRIMS(C1)
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
		int c = lower(*(w++));
		int x = btwi(c,'0','9') ? c-'0' : 99;
		if (btwi(c,'a','f')) { x = (c-'a'+10); }
		if (btwi(x, 0, b-1)) { n = (n*b)+x; } else { return 0; }
	}
	push(isNeg ? -n : n);
	return 1;
}

static int compileNumber(cell n) {
		if (btwi(n, 0, NUM_MASK)) { comma(n | NUM_BITS); }
		else { comma(LIT); comma(n); }
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
	if (strEqI(wd, ")"))  { return changeState(COMPILE); }
	if (strEqI(wd, "))")) { return changeState(INTERP); }
	if (state == COMMENT) { return 0; }
	if (strEqI(wd, ":"))  { return changeState(DEFINE); }
	if (strEqI(wd, ";"))  { comma(EXIT); return changeState(INTERP); }
	if (strEqI(wd, "["))  { return changeState(INTERP); }
	if (strEqI(wd, "]"))  { return changeState(COMPILE); }
	if (strEqI(wd, "("))  { return changeState(COMMENT); }
	if (strEqI(wd, "((")) { return changeState(COMMENT); }
	return 0;
}

void cfOuter(const char *src) {
	char *svIn = toIn;
	toIn = (char*)src;
	while (nextWord()) {
		if (isStateChange()) { continue; }
		if (state == COMMENT) { continue; }
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
		zType("-state?-");
		state = INTERP;
		break;
	}
	toIn = svIn;
}

void cfInit() {
	code  = (cell*)&mem[0];
	base  = 10;
	state = INTERP;
	last   = (cell)&mem[MEM_SZ-1];
	while (last & (CELL_SZ-1)) { --last; }
	dictEnd = last;
	vhere = dsp = rsp = lsp = tsp = asp = state = 0;
	here = BYE+1;
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
		{ "de-sz",   sizeof(DE_T) },   { "stk-sz",   STK_SZ+1 },
		{ "tstk-sz", TSTK_SZ+1 },      { "lstk-sz",  LSTK_SZ+1 },
		{ "memory",  (cell)&mem[0] },  { ">in",   (cell)&toIn },
		{ "mem-sz",  MEM_SZ },         { "base",  (cell)&base },
		{ "version", VERSION },        { "state",   (cell)&state },
		{ "cell",  CELL_SZ },          { 0 ,0 }
	};
	for (int i = 0; nvp[i].nm; i++) {
		DE_T *dp = addWord(nvp[i].nm);
		compileNumber(nvp[i].val);
		if (btwi(nvp[i].val, 0, NUM_MASK)) { dp->flags = _INLINE; }
		comma(EXIT);
	}

	#define D1(op, name, imm, code) { op, name, imm },
	PRIM_T prims[] = { PRIMS(D1) {0, 0, 0} };
	for (int i = 0; prims[i].name; i++) {
		DE_T *dp = addWord((char*)prims[i].name);
		dp->xt = prims[i].op;
		dp->flags = prims[i].fl;
	}
}

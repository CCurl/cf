// A ColorForth inspired system, MIT license

#include "cf.h"

#define NCASE         goto next; case
#define BCASE         break; case
#define TOS           dstk[dsp]
#define NOS           dstk[dsp-1]
#define L0            lstk[lsp]
#define L1            lstk[lsp-1]
#define L2            lstk[lsp-2]
#define DE_SZ         sizeof(DE_T)

byte mem[MEM_SZ];
cell dsp, dstk[STK_SZ+1];
cell rsp, rstk[STK_SZ+1];
cell lsp, lstk[LSTK_SZ+1];
cell tsp, tstk[TSTK_SZ+1];
cell asp, astk[TSTK_SZ+1];
cell base, state, dictEnd, outputFp;
cell *code, here, vhere, last;
char *toIn, wd[128];
DE_T tmpWords[10];

#define PRIMS \
	X(DUP,     "dup",       0, t=TOS; push(t); ) \
	X(SWAP,    "swap",      0, t=TOS; TOS=NOS; NOS=t; ) \
	X(DROP,    "drop",      0, pop(); ) \
	X(OVER,    "over",      0, t=NOS; push(t); ) \
	X(FET,     "@",         0, TOS = fetchCell((byte*)TOS); ) \
	X(STO,     "!",         0, t=pop(); n=pop(); storeCell((byte*)t, n); ) \
	X(WFET,    "w@",        0, TOS = fetchWord((byte*)TOS); ) \
	X(WSTO,    "w!",        0, t=pop(); n=pop(); storeWord((byte*)t, n); ) \
	X(CFET,    "c@",        0, TOS = *(byte *)TOS; ) \
	X(CSTO,    "c!",        0, t=pop(); n=pop(); *(byte*)t=(byte)n; ) \
	X(ADD,     "+",         0, t=pop(); TOS += t; ) \
	X(SUB,     "-",         0, t=pop(); TOS -= t; ) \
	X(MUL,     "*",         0, t=pop(); TOS *= t; ) \
	X(DIV,     "/",         0, t=pop(); TOS /= t; ) \
	X(SLMOD,   "/mod",      0, t=TOS; n = NOS; TOS = n/t; NOS = n%t; ) \
	X(INCR,    "1+",        0, ++TOS; ) \
	X(DECR,    "1-",        0, --TOS; ) \
	X(LT,      "<",         0, t=pop(); TOS = (TOS < t); ) \
	X(EQ,      "=",         0, t=pop(); TOS = (TOS == t); ) \
	X(GT,      ">",         0, t=pop(); TOS = (TOS > t); ) \
	X(EXIT,    "exit",      0, if (0<rsp) { pc = rpop(); } else { return; } ) \
	X(EQ0,     "0=",        0, TOS = (TOS == 0) ? 1 : 0; ) \
	X(AND,     "and",       0, t=pop(); TOS &= t; ) \
	X(OR,      "or",        0, t=pop(); TOS |= t; ) \
	X(XOR,     "xor",       0, t=pop(); TOS ^= t; ) \
	X(COM,     "com",       0, TOS = ~TOS; ) \
	X(FOR,     "for",       0, lsp+=3; L2=pc; L0=0; L1=pop(); ) \
	X(NDX_I,   "i",         0, push(L0); ) \
	X(NEXT,    "next",      0, if (++L0<L1) { pc=L2; } else { lsp=(2<lsp)?(lsp-3):0; } ) \
	X(TOR,     ">r",        0, rpush(pop()); ) \
	X(RSTO,    "r!",        0, rstk[rsp] = pop(); ) \
	X(RAT,     "r@",        0, push(rstk[rsp]); ) \
	X(RATI,    "r@+",       0, push(rstk[rsp]++); ) \
	X(RATD,    "r@-",       0, push(rstk[rsp]--); ) \
	X(RFROM,   "r>",        0, push(rpop()); ) \
	X(RDROP,   "rdrop",     0, rpop(); ) \
	X(TOT,     ">t",        0, if (tsp < STK_SZ) { tstk[++tsp] = pop(); } ) \
	X(TSTO,    "t!",        0, tstk[tsp] = pop(); ) \
	X(TAT,     "t@",        0, push(tstk[tsp]); ) \
	X(TATI,    "t@+",       0, push(tstk[tsp]++); ) \
	X(TATD,    "t@-",       0, push(tstk[tsp]--); ) \
	X(TFROM,   "t>",        0, push(0<tsp ? tstk[tsp--]: 0); ) \
	X(ATO,     ">a",        0, if (asp < STK_SZ) { astk[++asp] = pop(); } ) \
	X(ASET,    "a!",        0, astk[asp]=pop(); ) \
	X(AGET,    "a@",        0, push(astk[asp]); ) \
	X(AGETI,   "a@+",       0, push(astk[asp]++); ) \
	X(AGETD,   "a@-",       0, push(astk[asp]--); ) \
	X(AFROM,   "a>",        0, push(0<asp ? astk[asp--]: 0); ) \
	X(EMIT,    "emit",      0, emit((char)pop()); ) \
	X(KEY,     "key",       0, push(key()); ) \
	X(QKEY,    "?key",      0, push(qKey()); ) \
	X(OUTER,   "outer",     0, t=pop(); n=(cell)toIn; outer((char*)t); toIn=(char*)n; ) \
	X(ADDWORD, "addword",   0, addWord(0); ) \
	X(FIND,    "find",      0, { DE_T *dp=findWord(0); push(dp?dp->xt:0); push((cell)dp); } ) \
	X(CLK,     "timer",     0, push(timer()); ) \
	X(MS,      "ms",        0, ms(pop()); ) \
	X(ZTYPE,   "ztype",     0, zType((const char *)pop()); ) \
	X(FOPEN,   "fopen",     0, t=pop(); TOS=fOpen((char*)TOS, t); ) \
	X(FCLOSE,  "fclose",    0, t=pop(); fClose(t); ) \
	X(FREAD,   "fread",     0, t=pop(); n=pop(); TOS=fRead(TOS, n, t); ) \
	X(FWRITE,  "fwrite",    0, t=pop(); n=pop(); TOS=fWrite(TOS, n, t); ) \
	X(FSEEK,   "fseek",     0, t=pop(); n=pop(); fSeek(t,n); ) \
	X(SYSTEM,  "system",    0, t=pop(); ttyMode(0); system((char*)t); ) \
	X(SCOPY,   "s-cpy",     0, t=pop(); strCpy((char*)TOS, (char*)t); ) \
	X(SEQI,    "s-eqi",     0, t=pop(); n=pop(); push(strEqI((char*)n, (char*)t)); ) \
	X(SLEN,    "s-len",     0, TOS=strLen((char*)TOS); ) \
	X(BYE,     "bye",       0, ttyMode(0); exit(0); )

#define X(op, name, imm, cod) op,

enum _PRIM  {
	STOP, LIT, JMP, JMPZ, NJMPZ, JMPNZ, NJMPNZ, PRIMS
};

void push(cell x) { if (dsp < STK_SZ) { dstk[++dsp] = x; } }
cell pop() { return (0<dsp) ? dstk[dsp--] : 0; }
void rpush(cell x) { if (rsp < STK_SZ) { rstk[++rsp] = x; } }
cell rpop() { return (0<rsp) ? rstk[rsp--] : 0; }
int lower(const char c) { return btwi(c, 'A', 'Z') ? c+32 : c; }
int strLen(const char *s) { int l = 0; while (s[l]) { l++; } return l; }
void storeWord(byte *a, cell v) { *(ushort*)(a) = (ushort)v; }
ushort fetchWord(byte *a) { return *(ushort*)(a); }
void storeCell(byte *a, cell v) { *(cell*)(a) = v; }
cell fetchCell(byte *a) { return *(cell*)(a); }
void comma(cell n)    { code[here++] = n; }
int changeState(int newState) { state = newState; return newState; }
int checkWhitespace(char c) { return (btwi(c,DEFINE,COMMENT)) ? changeState(c) : 0; }

int strEqI(const char *s, const char *d) {
	while (lower(*s) == lower(*d)) { if (*s == 0) { return 1; } s++; d++; }
	return 0;
}

void strCpy(char *d, const char *s) {
	while (*s) { *(d++) = *(s++); }
	*(d) = 0;
}

int nextWord() {
	int len = 0;
	while (btwi(*toIn, 1, 32)) { checkWhitespace(*(toIn++)); }
	while (btwi(*toIn, 33, 126)) { wd[len++] = *(toIn++); }
	wd[len] = 0;
	return len;
}

int isTemp(const char *w) {
	return ((w[0] == 't') && btwi(w[1], '0', '9') && (w[2] == 0)) ? 1 : 0;
}

DE_T *addWord(char *w) {
	if (!w) { nextWord(); w = wd; }
	if (isTemp(w)) {
		tmpWords[w[1]-'0'].xt = (cell)here;
		return &tmpWords[w[1]-'0'];
	}
	int ln = strLen(w);
	if (ln > NAME_MAX) { ln=NAME_MAX; w[ln]=0; }
	last -= DE_SZ;
	DE_T *dp = (DE_T*)last;
	dp->xt = (cell)here;
	dp->flags = 0;
	dp->len = ln;
	strCpy(dp->name, w);
	// printf("\n-add:%lx,[%s],%d (%ld)-", last, dp->name, dp->len, dp->xt);
	return dp;
}

DE_T *findWord(const char *w) {
	if (!w) { nextWord(); w = wd; }
	if (isTemp(w)) { return &tmpWords[w[1] - '0']; }
	int len = strLen(w);
	cell cw = last;
	while (cw < dictEnd) {
		DE_T *dp = (DE_T*)cw;
		if ((len == dp->len) && strEqI(dp->name, w)) { return dp; }
		cw += DE_SZ;
	}
	return (DE_T*)0;
}

#undef X
#define X(op, name, imm, code) NCASE op: code

void inner(cell pc) {
	cell t, n, ir;
next:
	// printf("\n-pc:%d,ir:%d-",pc,ir);
	ir = code[pc++];
	switch(ir) {
		case  STOP:   return;
		NCASE LIT:    push(code[pc++]);
		NCASE JMP:    pc=code[pc];
		NCASE JMPZ:   t=code[pc++]; if (pop()==0) { pc = t; } // else { pc++; }
		NCASE NJMPZ:  t=code[pc++]; if (TOS==0)   { pc = t; } // else { pc++; }
		NCASE JMPNZ:  t=code[pc++]; if (pop())    { pc = t; } // else { pc++; }
		NCASE NJMPNZ: t=code[pc++]; if (TOS)      { pc = t; } // else { pc++; }
		PRIMS
		default:
			if ((ir & NUM_BITS) == NUM_BITS) { push(ir & NUM_MASK); goto next; }
			if (code[pc] != EXIT) { rpush(pc); }
			pc = ir;
			goto next;
	}
}

int isNumber(const char *w) {
	cell n=0, b=base, isNeg=0;
	if ((w[0]==39) && (w[2]==39) && (w[3]==0)) { push(w[1]); return 1; }
	if (w[0]=='#') { b=10; w++; }
	if (w[0]=='$') { b=16; w++; }
	if (w[0]=='%') { b=2; w++; }
	if (w[0]=='-') { isNeg=1; w++; }
	if (w[0]==0) { return 0; }
	char c = lower(*(w++));
	while (c) {
		if (btwi(c,'0','0'+b-1) && (c<='9')) { n = (n*b)+(c-'0'); }
		else if ((b==16) && btwi(c,'a','f')) { n = (n*b)+(c-'a'+10); }
		else { return 0; }
		c = lower(*(w++));
	}
	push(isNeg ? -n : n);
	return 1;
}

int compileNumber(cell n) {
		if (btwi(n, 0, NUM_MASK)) { comma(n | NUM_BITS); }
		else { comma(LIT); comma(n); }
		return 1;
}

void executeWord(DE_T *dp) {
	code[17] = dp->xt;
	code[18] = STOP;
	inner(17);
}

void compileWord(DE_T *dp) {
	if (dp->flags & 0x01) { executeWord(dp); }
	else if ((dp->flags & 0x02)) {   // Inline
		ucell x = dp->xt;
		do { comma(code[x++]); } while ( code[x] != EXIT );
	} else { comma(dp->xt); }
}

int isStateChange() {
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

int outer(const char *src) {
	char *svIn = toIn;
	toIn = (char*)src;
	while (nextWord()) {
		if (isStateChange()) { continue; }
		// printf("-wd:[%s],(%lld)-\n",wd,(int64_t)state);
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
	return 1;
}

void defNum(char *name, cell val) {
	DE_T *dp = addWord(name);
	compileNumber(val);
	if (btwi(val, 0, NUM_MASK)) { dp->flags = 0x02; }
	comma(EXIT);
}

#undef X
#define X(op, name, imm, code) { op, name, imm },

void baseSys() {
	PRIM_T prims[] = { PRIMS {0, 0, 0} };
	for (int i = 0; prims[i].name; i++) {
		DE_T *dp = addWord((char*)prims[i].name);
		dp->xt = prims[i].op;
		dp->flags = prims[i].fl;
	}
	here = BYE + 1;

	defNum("version",  VERSION);
	defNum("(jmp)",    JMP);
	defNum("(jmpz)",   JMPZ);
	defNum("(jmpnz)",  JMPNZ);
	defNum("(njmpz)",  NJMPZ);
	defNum("(njmpnz)", NJMPNZ);
	defNum("(lit)",    LIT);
	defNum("(exit)",   EXIT);
	defNum("(ztype)",  ZTYPE);

	defNum("(dsp)",  (cell)&dsp);
	defNum("dstk",   (cell)&dstk[0]);
	defNum("(rsp)",  (cell)&rsp);
	defNum("rstk",   (cell)&rstk[0]);
	defNum("(tsp)",  (cell)&tsp);
	defNum("tstk",   (cell)&tstk[0]);
	defNum("(asp)",  (cell)&asp);
	defNum("astk",   (cell)&astk[0]);
	defNum("(lsp)",  (cell)&lsp);
	defNum("lstk",   (cell)&lstk[0]);
	defNum("(ha)",   (cell)&here);
	defNum("(vha)",  (cell)&vhere);
	defNum("(la)",   (cell)&last);
	defNum("base",   (cell)&base);
	defNum("state",  (cell)&state);

	defNum("memory",      (cell)&mem[0]);
	defNum(">in",         (cell)&toIn);
	defNum("(output-fp)", (cell)&outputFp);

	defNum("mem-sz",   MEM_SZ);
	defNum("de-sz",    sizeof(DE_T));
	defNum("stk-sz",   STK_SZ+1);
	defNum("tstk-sz",  TSTK_SZ+1);
	defNum("lstk-sz",  LSTK_SZ+1);
	defNum("cell",     CELL_SZ);
}

void Init() {
	code	= (ucell*)&mem[0];
	base    = 10;
	state   = INTERP;
	last    = (cell)&mem[MEM_SZ-1];
	while (last & (CELL_SZ-1)) { --last; }
	dictEnd = last;
	vhere = dsp = rsp = lsp = tsp = asp = state = 0;
	baseSys();
}

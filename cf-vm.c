// A Tachyon inspired system, MIT license, (c) 2025 Chris Curl
#include "cf-vm.h"

#define X1(op, name, theCode) op,
#define X2(op, name, theCode) case op: theCode goto next;
#define X3(op, name, theCode) { name, op },

#define PRIMS(X) \
	X(EXIT,   "exit",     pc = (ucell)rpop(); if (pc==0) { return; } ) \
	X(LIT,    "",         push(code[pc++]); ) \
	X(JMP,    "",         pc = code[pc]; ) \
	X(JMPZ,   "",         if (pop()==0) { pc = code[pc]; } else { pc++; } ) \
	X(JMPNZ,  "",         if (pop()) { pc = code[pc]; } else { pc++; } ) \
	X(NJMPZ,  "",         if (TOS==0) { pc = code[pc]; } else { pc++; } ) \
	X(NJMPNZ, "",         if (TOS) { pc = code[pc]; } else { pc++; } ) \
	X(DUP,    "dup",      push(TOS); ) \
	X(DROP,   "drop",     pop(); ) \
	X(SWAP,   "swap",     t = TOS; TOS = NOS; NOS = t; ) \
	X(OVER,   "over",     push(NOS); ) \
	X(STO,    "!",        t = pop(); n = pop(); *(cell*)t = n; ) \
	X(FET,    "@",        TOS = *(cell*)TOS; ) \
	X(CSTO,   "c!",       t = pop(); n = pop(); *(byte*)t = (byte)n; ) \
	X(CFET,   "c@",       TOS = *(byte*)TOS; ) \
	X(RTO,    ">r",       rpush(pop()); ) \
	X(RAT,    "r@",       push(rstk[rsp]); ) \
	X(RFROM,  "r>",       push(rpop()); ) \
	X(ITSP,   "+L",       if (tsp < (STK_SZ-3)) { tsp += 3; } ) \
	X(DTSP,   "-L",       if (2 < tsp) { tsp -= 3; } ) \
	X(XSTO,   "x!",       tstk[tsp] = pop(); ) \
	X(YSTO,   "y!",       tstk[tsp+1] = pop(); ) \
	X(ZSTO,   "z!",       tstk[tsp+2] = pop(); ) \
	X(XFET,   "x@",       push(tstk[tsp]); ) \
	X(YFET,   "y@",       push(tstk[tsp+1]); ) \
	X(ZFET,   "z@",       push(tstk[tsp+2]); ) \
	X(MULT,   "*",        t = pop(); TOS *= t; ) \
	X(ADD,    "+",        t = pop(); TOS += t; ) \
	X(SUB,    "-",        t = pop(); TOS -= t; ) \
	X(SLMOD,  "/mod",     t = TOS; n = NOS; TOS = n/t; NOS = n%t; ) \
	X(INC,    "1+",       TOS += 1; ) \
	X(DEC,    "1-",       TOS -= 1; ) \
	X(LT,     "<",        t = pop(); TOS = (TOS  < t) ? 1 : 0; ) \
	X(EQ,     "=",        t = pop(); TOS = (TOS == t) ? 1 : 0; ) \
	X(GT,     ">",        t = pop(); TOS = (TOS  > t) ? 1 : 0; ) \
	X(EQ0,    "0=",       TOS = (TOS == 0) ? 1 : 0; ) \
	X(PLSTO,  "+!",       t = pop(); n = pop(); *(cell *)t += n; ) \
	X(FOR,    "for",      lsp += 3; L0 = 0; L1 = pop(); L2 = pc; ) \
	X(I,      "i",        push(L0); ) \
	X(NEXT,   "next",     if (++L0 < L1) { pc = (ucell)L2; } else { lsp = (2<lsp) ? lsp-3 : 0; } ) \
	X(AND ,   "and",      t = pop(); TOS &= t; ) \
	X(OR,     "or",       t = pop(); TOS |= t; ) \
	X(XOR,    "xor",      t = pop(); TOS ^= t; ) \
	X(ZTYPE,  "ztype",    zType((const char*)pop()); ) \
	X(FIND,   "find",     push((cell)findInDict((char *)0)); ) \
	X(KEY,    "key",      push(key()); ) \
	X(QKEY,   "key?",     push(qKey()); ) \
	X(EMIT,   "emit",     emit(pop()); ) \
	X(fOPEN,  "fopen",    t = pop(); TOS = fOpen(TOS, t); ) \
	X(fCLOSE, "fclose",   fClose(pop()); ) \
	X(fREAD,  "fread",    t = pop(); n = pop(); TOS = fRead(TOS, n, t); ) \
	X(fWRITE, "fwrite",   t = pop(); n = pop(); TOS = fWrite(TOS, n, t); ) \
	X(MS,     "ms",       ms(pop()); ) \
	X(TIMER,  "timer",    push(timer()); ) \
	X(ADDW,   "add-word", addToDict(0); ) \
	X(OUTER,  "outer",    t = pop(); outer((char*)t); ) \
	X(LASTOP, "system",   system((char*)pop()); )

enum { PRIMS(X1) };

char mem[MEM_SZ], *toIn, wd[32];
ucell *code=(ucell*)&mem[0], dsp, rsp, lsp, tsp, outputFp;
cell dstk[STK_SZ+1], rstk[STK_SZ+1], lstk[STK_SZ+1], tstk[STK_SZ+1];
cell here=LASTOP+1, last=(cell)&mem[MEM_SZ], base=10, state=INTERPRET;
DE_T tmpWords[10];

void push(cell v) { if (dsp < STK_SZ) { dstk[++dsp] = v; } }
cell pop() { return (0 < dsp) ? dstk[dsp--] : 0; }
void rpush(cell v) { if (rsp < STK_SZ) { rstk[++rsp] = v; } }
cell rpop() { return (0 < rsp) ? rstk[rsp--] : 0; }
void comma(ucell val) { code[here++] = val; }
void doComment() { while (nextWord() && !strEqI(wd, ")")) {} }
void doLineComment() { while ( *toIn && (*toIn != 10) ) { ++toIn; } }
void doNum() { if (state == COMPILE) { compileNum(pop()); } }
int  isTmpW(const char *w) { return (w[0]=='t') && btwi(w[1],'0','9') && (w[2]==0) ? 1 : 0; }
void addPrim(const char *nm, ucell op) { DE_T *dp = addToDict(nm); if (dp) { dp->xt = op; } }
void addLit(const char *name, cell val) { addToDict(name); compileNum(val); comma(EXIT); }
void doInline(ucell xt) { while (code[xt] != EXIT) { comma(code[xt++]); } }
void doInterp(ucell xt) { code[10]=xt; code[11]=EXIT; inner(10); }
char *checkWord(char *w) { return w ? w : (nextWord() ? &wd[0] : NULL); }

void compileNum(cell n) {
	if (btwi(n,0,LIT_BITS)) { comma((ucell)(n | LIT_MASK)); }
	else { comma(LIT); comma(n); }
}

int nextWord() {
	int ln = 0;
	while (*toIn && (*toIn < 33)) { ++toIn; }
	while (*toIn > 32) { wd[ln++] = *(toIn++); }
	wd[ln] = 0;
	return ln;
}

int isNum(const char *w, cell b) {
	cell n = 0, isNeg = 0;
	if ((w[0] == 39) && (w[2] == 39) && (w[3] == 0)) { push(w[1]); return 1; }
	if (w[0] == '%') { b = 2; ++w; }
	if (w[0] == '#') { b = 10; ++w; }
	if (w[0] == '$') { b = 16; ++w; }
	if ((b == 10) && (w[0] == '-')) { isNeg = 1; ++w; }
	if (w[0] == 0) { return 0; }
	while (*w) {
		char x = *(w++), c = btwi(x,'A','Z') ? x+32 : x;
		if ((btwi(c,'0','9')) && btwi(c,'0','0'+b-1)) { n = (n*b)+(c-'0'); }
		else if (btwi(c,'a','a'+b-11)) { n = (n*b)+(c-'a'+10); }
		else return 0;
	}
	push(isNeg ? -n : n);
	return 1;
}

DE_T *addToDict(const char *w) {
	w = checkWord((char*)w);
	if (isTmpW(w)) { DE_T *x = &tmpWords[w[1]-'0']; x->xt = here; return x; }
	int ln = strlen(w);
	if (ln == 0) { return (DE_T*)0; }
	byte sz = CELL_SZ + 3 + ln + 1; // xt, sz, fl, ln, name[], null
	while (sz & 0x03) { ++sz; }
	last -= sz;
	DE_T *dp = (DE_T*)last;
	dp->xt = (ucell)here;
	dp->sz = sz;
	dp->fl = 0;
	dp->ln = ln;
	strcpy(dp->nm, w);
	return dp;
}

DE_T *findInDict(char *w) {
	w = checkWord((char*)w);
	if (isTmpW(w)) { return &tmpWords[w[1]-'0']; }
	cell cw = last, ln = strlen(w);
	while (cw < (cell)&mem[MEM_SZ]) {
		DE_T *dp = (DE_T *)cw;
		if ((dp->ln == ln) && (strEqI(dp->nm, w))) { return dp; }
		cw += dp->sz;
	}
	return (DE_T *)0;
}

void inner(ucell pc) {
	cell n, t, ir;
next: ir = code[pc++];
	switch (ir)	{
		PRIMS(X2)
	default:
		if ((ir & LIT_MASK) == LIT_MASK) { push(ir & LIT_BITS); goto next; }
		if (code[pc] != EXIT) { rpush(pc); }
		pc = ir;
		goto next;
	}
}

void outer(const char *src) {
	char *svIn = toIn;
	toIn = (char *)src;
	while (nextWord() && (state != BYE)) {
		if (strEqI(wd, "("))  { doComment(); continue; }
		if (strEqI(wd, "\\")) { doLineComment(); continue; }
		if (strEqI(wd, ";"))  { state=INTERPRET; comma(EXIT); continue; }
		if (strEqI(wd, ":"))  { state=COMPILE; addToDict(0); continue; }
		if (isNum(wd, base))  { doNum(); continue; }
		DE_T *dp = findInDict(wd);
		if (!dp) {
			zType("\n-word:["); zType(wd); zType("]?-\n");
			state = INTERPRET;
			break;
		}
		if ((state == INTERPRET) || (dp->fl & IMMED)) { doInterp(dp->xt); }
		else { (dp->fl & INLINE) ? doInline(dp->xt) : comma(dp->xt); } // COMPILE
	}
	toIn = svIn;
}

void cfInit() {
	NVP_T prims[] = { PRIMS(X3) { 0, 0 } };
	NVP_T nv[] = {
		{ "version", VERSION },        { "output-fp", (cell)&outputFp },
		{ "(h)",     (cell)&here },    { "(l)",       (cell)&last },
		{ "(lsp)",   (cell)&lsp },     { "lstk",      (cell)&lstk[0] },
		{ "(rsp)",   (cell)&rsp },     { "rstk",      (cell)&rstk[0] },
		{ "(tsp)",   (cell)&tsp },     { "tstk",      (cell)&tstk[0] },
		{ "(sp)",    (cell)&dsp },     { "stk",       (cell)&dstk[0] },
		{ "state",   (cell)&state },   { "base",      (cell)&base },
		{ "mem",     (cell)&mem[0] },  { "mem-sz",    (cell)MEM_SZ },
		{ ">in",     (cell)&toIn},     { "cell",      CELL_SZ }, { 0, 0 }
	};
	for (int i = 0; nv[i].name; i++) { addLit(nv[i].name, nv[i].value); }
	for (int i = 0; prims[i].name; i++) { addPrim(prims[i].name, prims[i].value); }
}

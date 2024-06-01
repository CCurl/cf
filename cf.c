#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include "cf.h"

#define LASTPRIM      BYE
#define NCASE         goto next; case
#define BCASE         break; case
#define here          code[0]
#define last          code[1]
#define vhere         code[2]
#define base          code[3]
#define state         code[4]
#define lex           code[5]
#define TOS           stk[sp].i
#define NOS           stk[sp-1].i
#define L0            lstk[lsp]
#define L1            lstk[lsp-1]
#define L2            lstk[lsp-2]

SE_T stk[STK_SZ+1];
ushort code[CODE_SZ+1];
byte dict[DICT_SZ+1], vars[VARS_SZ+1];
short sp, rsp, lsp, aSp;
cell A, B, S, D, lstk[LSTK_SZ], rstk[STK_SZ+1];
char tib[128], wd[32], *toIn, wordAdded;

#define PRIMS \
	X(EXIT,    "exit",      0, if (0<rsp) { pc = (ushort)rpop(); } else { return; } ) \
	X(DUP,     "dup",       0, t=TOS; push(t); ) \
	X(SWAP,    "swap",      0, t=TOS; TOS=NOS; NOS=t; ) \
	X(DROP,    "drop",      0, pop(); ) \
	X(OVER,    "over",      0, t=NOS; push(t); ) \
	X(FET,     "@",         0, TOS = fetchCell(TOS); ) \
	X(CFET,    "c@",        0, TOS = *(byte *)TOS; ) \
	X(WFET,    "w@",        0, TOS = fetchWord(TOS); ) \
	X(STO,     "!",         0, t=pop(); n=pop(); storeCell(t, n); ) \
	X(CSTO,    "c!",        0, t=pop(); n=pop(); *(byte*)t=(byte)n; ) \
	X(WSTO,    "w!",        0, t=pop(); n=pop(); storeWord(t, n); ) \
	X(ADD,     "+",         0, t=pop(); TOS += t; ) \
	X(SUB,     "-",         0, t=pop(); TOS -= t; ) \
	X(MUL,     "*",         0, t=pop(); TOS *= t; ) \
	X(DIV,     "/",         0, t=pop(); TOS /= t; ) \
	X(SLMOD,   "/mod",      0, t=TOS; n = NOS; TOS = n/t; NOS = n%t; ) \
	X(INC,     "1+",        0, ++TOS; ) \
	X(DEC,     "1-",        0, --TOS; ) \
	X(LT,      "<",         0, t=pop(); TOS = (TOS < t); ) \
	X(EQ,      "=",         0, t=pop(); TOS = (TOS == t); ) \
	X(GT,      ">",         0, t=pop(); TOS = (TOS > t); ) \
	X(EQ0,     "0=",        0, TOS = (TOS == 0) ? 1 : 0; ) \
	X(AND,     "and",       0, t=pop(); TOS &= t; ) \
	X(OR,      "or",        0, t=pop(); TOS |= t; ) \
	X(XOR,     "xor",       0, t=pop(); TOS ^= t; ) \
	X(COM,     "com",       0, TOS = ~TOS; ) \
	X(FOR,     "for",       0, lsp+=3; L2=pc; L0=0; L1=pop(); ) \
	X(INDEX,   "i",         0, push(L0); ) \
	X(NEXT,    "next",      0, if (++L0<L1) { pc=(ushort)L2; } else { lsp=(lsp<3) ? 0 : lsp-3; } ) \
	X(AGET,    "a",         0, push(A); ) \
	X(ASET,    ">a",        0, A=pop(); ) \
	X(AINC,    "a+",        0, push(A++); ) \
	X(BGET,    "b",         0, push(B); ) \
	X(BINC,    "b+",        0, push(B++); ) \
	X(BSET,    ">b",        0, B=pop(); ) \
	X(SGET,    "s",         0, push(S); ) \
	X(SINC,    "s+",        0, push(S++); ) \
	X(SSET,    ">s",        0, S=pop(); ) \
	X(DGET,    "d",         0, push(D); ) \
	X(DINC,    "d+",        0, push(D++); ) \
	X(DSET,    ">d",        0, D=pop(); ) \
	X(TOR,     ">r",        0, rpush(pop()); ) \
	X(RAT,     "r@",        0, push(rstk[rsp]); ) \
	X(RFROM,   "r>",        0, push(rpop()); ) \
	X(EMIT,    "emit",      0, t=pop(); emit((char)t); ) \
	X(CLK,     "timer",     0, push(clock()); ) \
	X(SEE,     "see",       0, doSee(); ) \
	X(COUNT,   "count",     0, t=pop(); push(t+1); push(*(byte *)t); ) \
	X(TYPE,    "type",      0, t=pop(); char *y=(char*)pop(); for (int i = 0; i<t; i++) emit(y[i]); ) \
	X(QUOTE,   "\"",        0, quote(); ) \
	X(DOTQT,   ".\"",       0, quote(); comma(COUNT); comma(TYPE); ) \
	X(RAND,    "rand",      0, doRand(); ) \
	X(FLOPEN,  "fopen",     0, t=pop(); n=pop(); push(fileOpen((char*)n, (char*)t)); ) \
	X(FLCLOSE, "fclose",    0, t=pop(); fileClose(t); ) \
	X(FLREAD,  "fread",     0, t=pop(); n=pop(); TOS = fileRead((char*)TOS, n, t); ) \
	X(FLWRITE, "fwrite",    0, t=pop(); n=pop(); TOS = fileWrite((char*)TOS, n, t); ) \
	X(FLGETS,  "fgets",     0, t=pop(); n=pop(); TOS = fileGets((char*)TOS, (int)n, t); ) \
	X(FLLOAD,  "fload",     0, t=pop(); fileLoad((char*)t); ) \
	X(LOAD,    "load",      0, t=pop(); blockLoad((int)t); ) \
	X(LOADED,  "loaded?",   0, t=pop(); pop(); if (t) { fileClose(inputFp); inputFp=filePop(); } ) \
	X(ITOA,    "to-string", 0, t=pop(); push((cell)iToA(t, base)); ) \
	X(DOTS,    ".s",        0, dotS(); ) \
	X(FETC,    "@c",        0, TOS = code[(ushort)TOS]; ) \
	X(STOC,    "!c",        0, t=pop(); n=pop(); code[(ushort)t] = (ushort)n; /**/) \
	X(FIND,    "find",      0, { DE_T *dp = (DE_T*)findWord(0); push(dp?dp->xt:0); push((cell)dp); } ) \
	X(SYSTEM,  "system",    0, t=pop(); system((char*)t+1); ) \
	X(BYE,     "bye",       0, exit(0); )

#define X(op, name, imm, cod) op,

enum _PRIM  {
	STOP, LIT1, LIT2, JMP, JMPZ, JMPNZ,
	PRIMS
};

#undef X
#define X(op, name, imm, cod) { op, name, imm },

typedef struct { short op; const char *name; byte imm; } PRIM_T;
PRIM_T prims[] = {
	PRIMS
	{0, 0, 0}
};

void sys_load();
void push(cell x) { if (sp < STK_SZ) { stk[++sp].i = x; } }
cell pop() { return (0<sp) ? stk[sp--].i : 0; }
void rpush(cell x) { if (rsp < RSTK_SZ) { rstk[++rsp] = x; } }
cell rpop() { return (0<rsp) ? rstk[rsp--] : 0; }
void storeCell(cell a, cell val) { *(cell*)(a) = val; }
void storeWord(cell a, cell val) { *(ushort*)(a) = (ushort)val; }
cell fetchCell(cell a) { return *(cell*)(a); }
cell fetchWord(cell a) { return *(ushort*)(a); }
int lower(char c) { return btwi(c, 'A', 'Z') ? c + 32 : c; }
int strLen(const char *s) { int l = 0; while (s[l]) { l++; } return l; }
void emit(char c) { fputc(c, outputFp ? (FILE*)outputFp : stdout); }
int changeState(char c) { state = c; return c; }
void printString(char *s) { fputs(s, outputFp ? (FILE*)outputFp : stdout); }

int strEq(const char *s, const char *d) {
	while (*s == *d) { if (*s == 0) { return 1; } s++; d++; }
	return 0;
}

int strEqI(const char *s, const char *d) {
	while (lower(*s) == lower(*d)) { if (*s == 0) { return 1; } s++; d++; }
	return 0;
}

void strCpy(char *d, const char *s) {
	while (*s) { *(d++) = *(s++); }
	*(d) = 0;
}

void comma(x) { code[here++] = x; }
void commaCell(cell n) {
	storeCell((cell)&code[here], n);
	here += sizeof(cell) / 2;
}

void checkState(char c) {
	if (btwi(c, RED, WHITE)) { state = c; }
}

int nextWord() {
	int l = 0;
	while (btwi(*toIn, 1, 32)) { checkState(*(toIn++)); }
	while (btwi(*toIn, 33, 126)) { wd[l++] = *(toIn++); }
	wd[l] = 0;
	return l;
}

DE_T *addWord(const char *w) {
	if (!w) { nextWord(); w = wd; }
	int ln = strLen(w);
	int sz = ln + 7;          // xt + sz + fl + lx + ln + null
	if (sz & 1) { ++sz; }
	ushort newLast=last - sz;
	DE_T *dp = (DE_T*)&dict[newLast];
	dp->sz = sz;
	dp->xt=here;
	dp->fl = 0;
	dp->lx = (byte)lex;
	dp->ln = ln;
	strCpy(dp->nm, w);
	last=newLast;
	// printf("-add:%d,[%s],%d (%d)-\n", newLast, dp->nm, dp->lx, dp->xt);
	return dp;
}

DE_T *findWord(const char *w) {
	if (!w) { nextWord(); w = wd; }
	int len = strLen(w);
	int cw = last;
	while (cw < DICT_SZ) {
		DE_T *dp = (DE_T*)&dict[cw];
		// printf("-%d,(%s)-", cw, dp->nm);
		if ((len == dp->ln) && strEqI(dp->nm, w)) { return dp; }
		cw += dp->sz;
	}
	return (DE_T*)0;
}

int findXT(int xt) {
	int cw = last;
	while (cw < DICT_SZ) {
		DE_T *dp = (DE_T*)&dict[cw];
		// printf("-%d,(%s)-", cw, dp->nm);
		if (dp->xt == xt) { return cw; }
		cw += dp->sz;
	}
	return 0;
}

int findPrevXT(int xt) {
	int prevXT=here;
	int cw = last;
	while (cw < DICT_SZ) {
		DE_T *dp = (DE_T*)&dict[cw];
		if (dp->xt == xt) { return prevXT; }
		prevXT=dp->xt;
		cw += dp->sz;
	}
	return here;
}

void doSee() {
	DE_T *dp = findWord(0);
	if (!dp) { printf("-nf:%s-", wd); return; }
	if (dp->xt < LASTPRIM) { printf("%s is a primitive (%hX).\n", wd, dp->xt); return; }
	cell x = (cell)dp-(cell)dict;
	int stop = findPrevXT(dp->xt)-1;
	int i = dp->xt;
	printf("\n%04lX: %s (%04hX to %04X)", x, dp->nm, dp->xt, stop);
	while (i <= stop) {
		int op = code[i++];
		x = code[i];
		printf("\n%04X: %04X\t", i-1, op);
		switch (op) {
			case  STOP: printf("stop"); i++;
			BCASE LIT1: printf("lit1 %ld (%lX)", x, x); i++;
			BCASE LIT2: x = fetchCell((cell)&code[i]);
				printf("lit2 %ld (%lX)", x, x);
				i += CELL_SZ / 2;
			BCASE JMP:   printf("jmp %04lX", x);   i++;
			BCASE JMPZ:  printf("jmpz %04lX (IF)", x);     i++;
			BCASE JMPNZ: printf("jmpnz %04lX (WHILE)", x); i++; break;
			default: x = findXT(op); 
				printf("%s", x ? ((DE_T*)&dict[(ushort)x])->nm : "??");
		}
	}
}

char *iToA(cell N, int b) {
	static char buf[65];
	ucell X = (ucell)N;
	int isNeg = 0, len = 0;
	if (b == 0) { b = (int)base; }
	if ((b == 10) && (N < 0)) { isNeg = 1; X = -N; }
	char c, *cp = &buf[64];
	*(cp) = 0;
	do {
		c = (char)(X % b) + '0';
		X /= b;
		c = (c > '9') ? c+7 : c;
		*(--cp) = c;
		++len;
	} while (X);
	if (isNeg) { *(--cp) = '-'; }
	*(--cp) = len;
	return cp;
}

void dotS() {
	printf("( ");
	for (int i = 1; i <= sp; i++) { printf("%s ", iToA(stk[i].i, base)+1); }
	printf(")");
}

void quote() {
	comma(LIT2);
	commaCell((cell)&vars[vhere]);
	ushort start = vhere;
	vars[vhere++] = 0; // Length byte
	if (*toIn) { ++toIn; }
	while (*toIn) {
		if (*toIn == '"') { ++toIn; break; }
		vars[vhere++] = *(toIn++);
		++vars[start];
	}
	vars[vhere++] = 0; // NULL terminator
}

void dotQuote() {
	quote(); comma(COUNT); comma(TYPE);
}

void doRand() {
	static cell sd = 0;
	if (!sd) { sd = (cell)code  *clock(); }
	sd = (sd << 13) ^ sd;
	sd = (sd >> 17) ^ sd;
	sd = (sd <<  5) ^ sd;
	push(sd);
}

int isNum(const char *w, int b) {
	cell n=0, isNeg=0;
	if ((w[0]==39) && (w[2]==39) && (w[3]==0)) { push(w[1]); return 1; }
	if (w[0]=='%') { b= 2; ++w; }
	if (w[0]=='#') { b=10; ++w; }
	if (w[0]=='$') { b=16; ++w; }
	if ((b==10) && (w[0]=='-')) { isNeg=1; ++w; }
	if (w[0]==0) { return 0; }
	char c = lower(*(w++));
	while (c) {
		n = (n*b);
		if (btwi(c,'0','9') &&  btwi(c,'0','0'+b-1)) { n+=(c-'0'); }
		else if (btwi(c,'a','a'+b-11)) { n+=(c-'a'+10); }
		else return 0;
		c = lower(*(w++));
	}
	if (isNeg) { n = -n; }
	push(n);
	return 1;
}

#undef X
#define X(op, name, imm, code) NCASE op: code

void Exec(int start) {
	cell t, n;
	ushort pc = start, wc;
	next:
	wc = code[pc++];
	switch(wc) {
		case  STOP:  return;
		NCASE LIT1:  push(code[pc++]);
		NCASE LIT2:  push(fetchCell((cell)&code[pc])); pc += CELL_SZ/2;
		NCASE JMP:   pc=code[pc];
		NCASE JMPZ:  if (pop()==0) { pc=code[pc]; } else { ++pc; }
		NCASE JMPNZ: if (pop()) { pc=code[pc]; } else { ++pc; }
		PRIMS
		default: if (code[pc] != EXIT) { rpush(pc); }
				 pc = wc;
				 goto next;
	}
}

int doCompile(char *w) {
	if (isNum(w, 10)) {
		cell n = pop();
		if (btwi(n, 0, 0xffff)) {
			comma(LIT1); comma((ushort)n);
		} else {
			comma(LIT2);
			commaCell(n);
		}
		return 1;
	}

	DE_T *de = findWord(w);
	if (de) {
		comma(de->xt);
		return 1;
	}
	printf("-compile:%s?-",wd);
	return 0;
}

int doInterpret(char *w) {
	if (isNum(w, 10)) {
		return 1;
	}

	DE_T *de = findWord(w);
	if (de) {
		int h = here+100;
		code[h]   = de->xt;
		code[h+1] = EXIT;
		Exec(h);
		return 1;
	}
	printf("-interpret:%s?-",wd);
	return 0;
}

int setState(char *wd) {
	if (strEq(wd, ":D")) { return changeState(DEFINE);  }
	if (strEq(wd, ":I")) { return changeState(INLINE);  }
	if (strEq(wd, ":M")) { return changeState(MLMODE);  }
	if (strEq(wd, "["))  { return changeState(INTERP);  }
	if (strEq(wd, "]"))  { return changeState(COMPILE); }
	if (strEq(wd, "("))  { return changeState(COMMENT); }
	if (strEq(wd, ")"))  { return changeState(COMPILE); }
	if (strEq(wd, "((")) { return changeState(COMMENT); }
	if (strEq(wd, "))")) { return changeState(INTERP);  }

	// Auto state transitions for text-based usage
	if (strEq(wd, ":"))  { addWord(0);  return changeState(COMPILE); }
	if (strEq(wd, ";"))  { comma(EXIT); return changeState(INTERP); }
	return 0;
}

void doOuter(const char *src) {
	DE_T *dp;
	toIn = (char*)src;
	while (nextWord()) {
		if (setState(wd)) { continue; }
		// printf("-%d:%s-\n",state,wd);
		switch (state) {
			case COMMENT:
			BCASE DEFINE:  addWord(wd);
			BCASE INLINE:  dp = addWord(wd); // last->f=IS_INLINE;
			BCASE COMPILE: if (!doCompile(wd)) return;
			BCASE INTERP:  if (!doInterpret(wd)) return;
			// BCASE MLMODE:  if (!doML()) return;
			break; default: printString("-state?-"); break;
		}
	}
}

void parseF(const char *fmt, ...) {
	char buf[128];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, 128, fmt, args);
	va_end(args);
	doOuter(buf);
}

void sys_load() {
	blockLoad(123);
}

void baseSys() {
	for (int i = 0; prims[i].name; i++) {
		DE_T *w = addWord(prims[i].name);
		w->xt=prims[i].op;
		w->fl = prims[i].imm;
	}

	parseF(": version   #%d ;", VERSION);
	parseF(": (jmp)     #%d ;", JMP);
	parseF(": (jmpz)    #%d ;", JMPZ);
	parseF(": (jmpnz)   #%d ;", JMPNZ);
	parseF(": (lit1)    #%d ;", LIT1);
	parseF(": (lit2)    #%d ;", LIT2);
	parseF(": (exit)    #%d ;", EXIT);

	parseF(": (here)    #%d ;", 0);
	parseF(": (last)    #%d ;", 1);
	parseF(": (vhere)   #%d ;", 2);
	parseF(": base      #%d ;", 3);
	parseF(": state     #%d ;", 4);
	parseF(": (lex)     #%d ;", 5);

	parseF(addrFmt, "code", &code[0]);
	parseF(addrFmt, "vars", &vars[0]);
	parseF(addrFmt, "dict", &dict[0]);
	parseF(addrFmt, ">in",  &toIn);

	parseF(": code-sz #%d ;", CODE_SZ);
	parseF(": vars-sz #%d ;", VARS_SZ);
	parseF(": dict-sz #%d ;", DICT_SZ);
	parseF(": cell    #%d ;", CELL_SZ);
	sys_load();
}

void Init() {
	for (int t=0; t<CODE_SZ; t++) { code[t]=0; }
	for (int t=0; t<VARS_SZ; t++) { vars[t]=0; }
	for (int t=0; t<DICT_SZ; t++) { dict[t]=0; }
	sp = rsp = lsp = aSp = state = 0;
	last=DICT_SZ;
	base = 10;
	here = LASTPRIM+1;
	fileInit();
	baseSys();
}

// REP - Read/Execute/Print (no Loop)
void REP() {
	if (inputFp == 0) {
		if (state==INTERP) { printString(" ok\n"); }
		else { printString(" ... "); }
	}
	if (fileGets(tib, sizeof(tib), inputFp)) {
		doOuter(tib+1);
		return;
	}
	if (inputFp == 0) { exit(0); }
	fileClose(inputFp);
	inputFp = filePop();
}

int main(int argc, char *argv[]) {
	Init();
	if (argc>1) {
		// load init block first (if necessary)
		cell tmp = fileOpen(argv[1]-1, " rt");
		if (tmp && inputFp) { filePush(tmp); }
		else { inputFp = tmp; }
	}
	while (1) { REP(); };
	return 0;
}

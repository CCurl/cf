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
cell A, B, S, D, lstk[LSTK_SZ], rstk[RSTK_SZ+1];
char tib[128], wd[32], *toIn, wordAdded;

#define BOARDPRIMS

#define FILEPRIMS \
	X(BLOCK,   "block",     TOS=blockData(TOS); ) \
	X(DIRTY,   "dirty",     t=pop(); n=pop(); blockDirty((int)n, (int)t); ) \
	X(DIRTYQ,  "dirty?",    TOS = blockDirtyQ((int)TOS); ) \
	X(FLUSH,   "flush",     flushBlocks(); ) \
	X(FLOPEN,  "fopen",     t=pop(); n=pop(); push(fileOpen((char*)n, (char*)t)); ) \
	X(FLCLOSE, "fclose",    t=pop(); fileClose(t); ) \
	X(FLREAD,  "fread",     t=pop(); n=pop(); TOS = fileRead((byte*)TOS, n, t); ) \
	X(FLWRITE, "fwrite",    t=pop(); n=pop(); TOS = fileWrite((byte*)TOS, n, t); ) \
	X(FLGETS,  "fgets",     t=pop(); n=pop(); TOS = fileGets((char*)TOS, (int)n, t); ) \
	X(FLLOAD,  "fload",     t=pop(); fileLoad((char*)t); ) \
	X(LOAD,    "load",      t=pop(); blockLoad((int)t); ) \
	X(LOADED,  "loaded?",   t=pop(); pop(); if (t) { fileClose(inputFp); inputFp=filePop(); } )

#define PRIMS \
	X(EXIT,    "exit",      if (0<rsp) { pc = (ushort)rpop(); } else { return; } ) \
	X(DUP,     "dup",       t=TOS; push(t); ) \
	X(SWAP,    "swap",      t=TOS; TOS=NOS; NOS=t; ) \
	X(DROP,    "drop",      pop(); ) \
	X(OVER,    "over",      t=NOS; push(t); ) \
	X(FET,     "@",         TOS = fetchCell(TOS); ) \
	X(CFET,    "c@",        TOS = *(byte *)TOS; ) \
	X(WFET,    "w@",        TOS = fetchWord(TOS); ) \
	X(STO,     "!",         t=pop(); n=pop(); storeCell(t, n); ) \
	X(CSTO,    "c!",        t=pop(); n=pop(); *(byte*)t=(byte)n; ) \
	X(WSTO,    "w!",        t=pop(); n=pop(); storeWord(t, n); ) \
	X(ADD,     "+",         t=pop(); TOS += t; ) \
	X(SUB,     "-",         t=pop(); TOS -= t; ) \
	X(MUL,     "*",         t=pop(); TOS *= t; ) \
	X(DIV,     "/",         t=pop(); TOS /= t; ) \
	X(SLMOD,   "/mod",      t=TOS; n = NOS; TOS = n/t; NOS = n%t; ) \
	X(INC,     "1+",        ++TOS; ) \
	X(DEC,     "1-",        --TOS; ) \
	X(LT,      "<",         t=pop(); TOS = (TOS < t); ) \
	X(EQ,      "=",         t=pop(); TOS = (TOS == t); ) \
	X(GT,      ">",         t=pop(); TOS = (TOS > t); ) \
	X(EQ0,     "0=",        TOS = (TOS == 0) ? 1 : 0; ) \
	X(AND,     "and",       t=pop(); TOS &= t; ) \
	X(OR,      "or",        t=pop(); TOS |= t; ) \
	X(XOR,     "xor",       t=pop(); TOS ^= t; ) \
	X(COM,     "com",       TOS = ~TOS; ) \
	X(FOR,     "for",       lsp+=3; L2=pc; L0=0; L1=pop(); ) \
	X(INDEX,   "i",         push(L0); ) \
	X(NEXT,    "next",      if (++L0<L1) { pc=(ushort)L2; } else { lsp=(lsp<3) ? 0 : lsp-3; } ) \
	X(AGET,    "a",         push(A); ) \
	X(ASET,    ">a",        A=pop(); ) \
	X(AINC,    "a+",        push(A++); ) \
	X(BGET,    "b",         push(B); ) \
	X(BINC,    "b+",        push(B++); ) \
	X(BSET,    ">b",        B=pop(); ) \
	X(SGET,    "s",         push(S); ) \
	X(SINC,    "s+",        push(S++); ) \
	X(SSET,    ">s",        S=pop(); ) \
	X(DGET,    "d",         push(D); ) \
	X(DINC,    "d+",        push(D++); ) \
	X(DSET,    ">d",        D=pop(); ) \
	X(TOR,     ">r",        rpush(pop()); ) \
	X(RAT,     "r@",        push(rstk[rsp]); ) \
	X(RFROM,   "r>",        push(rpop()); ) \
	X(EMIT,    "emit",      t=pop(); emit((char)t); ) \
	X(CLK,     "timer",     push(clock()); ) \
	X(SEE,     "see",       doSee(); ) \
	X(COUNT,   "count",     t=pop(); push(t+1); push(*(byte *)t); ) \
	X(TYPE,    "type",      t=pop(); char *y=(char*)pop(); for (int i = 0; i<t; i++) emit(y[i]); ) \
	X(ADDW,    "add-word",  addWord(0); ) \
	X(QUOTE,   "\"",        quote(); ) \
	X(DOTQT,   ".\"",       quote(); comma(COUNT); comma(TYPE); ) \
	X(RAND,    "rand",      doRand(); ) \
	X(ITOA,    "to-string", t=pop(); push((cell)iToA(t, base)); ) \
	X(DOTS,    ".s",        dotS(); ) \
	X(FETC,    "@c",        TOS = code[(ushort)TOS]; ) \
	X(STOC,    "!c",        t=pop(); n=pop(); code[(ushort)t] = (ushort)n; /**/) \
	X(FIND,    "find",      { DE_T *dp = (DE_T*)findWord(0); push(dp?dp->xt:0); push((cell)dp); } ) \
	X(SYSTEM,  "system",    t=pop(); system((char*)t+1); ) \
	X(STREQ,   "streq",     t=pop(); TOS=strEq((char*)t, (char*)TOS); ) \
	X(STREQI,  "streqi",    t=pop(); TOS=strEqI((char*)t, (char*)TOS); ) \
	X(STRCPY,  "strcpy",    t=pop(); n=pop(); strCpy((char*)n, (char*)t); ) \
	X(STRLEN,  "strlen",    TOS=strlen((char*)TOS); ) \
	FILEPRIMS BOARDPRIMS \
	X(BYE,     "bye",       exit(0); )

#define X(op, name, cod) op,

enum _PRIM  {
	STOP, LIT1, LIT2, JMP, JMPZ, NJMPZ, JMPNZ, 
	PRIMS
};

#undef X
#define X(op, name, cod) { op, name },

typedef struct { short op; const char *name; } PRIM_T;
PRIM_T prims[] = {
	PRIMS
	{0, 0}
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
void emit(char c) { fputc(c, outputFp ? (FILE*)outputFp : stdout); }
int changeState(char c) { state = c; return c; }
void printString(char *s) { fputs(s, outputFp ? (FILE*)outputFp : stdout); }
int strLen(const char *s) { int l = 0; while (s[l]) { l++; } return l; }
int lower(char c) { return btwi(c, 'A', 'Z') ? c + 32 : c; }

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

void fill(byte *addr, cell num, byte ch) {
	while (num--) { *(addr++) = ch; }
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
	if (dp->xt < LASTPRIM) { printf("%s is a primitive ($%hX).\n", wd, dp->xt); return; }
	cell x = (cell)dp-(cell)dict;
	int stop = findPrevXT(dp->xt)-1;
	int i = dp->xt;
	printf("\n%04lX: %s ($%04hX to $%04X)", x, dp->nm, dp->xt, stop);
	while (i <= stop) {
		int op = code[i++];
		x = code[i];
		printf("\n%04X: %04X\t", i-1, op);
		switch (op) {
			case  STOP: printf("stop"); i++;
			BCASE LIT1: printf("lit1 #%ld ($%lX)", x, x); i++;
			BCASE LIT2: x = fetchCell((cell)&code[i]);
				printf("lit2 #%ld ($%lX)", x, x);
				i += CELL_SZ / 2;
			BCASE JMP:   printf("jmp %04lX", x);   i++;
			BCASE JMPZ:  printf("jmpz %04lX (if)", x);     i++;
			BCASE NJMPZ: printf("njmpz %04lX (-if)", x);   i++;
			BCASE JMPNZ: printf("jmpnz %04lX (while)", x); i++; break;
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
#define X(op, name, code) NCASE op: code

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
		NCASE NJMPZ: if (TOS==0)   { pc=code[pc]; } else { ++pc; }
		NCASE JMPNZ: if (pop())    { pc=code[pc]; } else { ++pc; }
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
		int h = BYE-3;
		code[h] = de->xt;
		code[h+1] = EXIT;
		Exec(h);
		return 1;
	}
	printf("-interpret:%s?-",wd);
	return 0;
}

int setState(char *wd) {
	if (strEq(wd, ":D")) { return changeState(DEFINE);  }
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
	int isErr = 0;
	toIn = (char*)src;
	while (nextWord()) {
		if (setState(wd)) { continue; }
		// printf("-%d:%s-\n",state,wd);
		switch (state) {
			case COMMENT:
			BCASE DEFINE:  addWord(wd);
			BCASE COMPILE: if (doCompile(wd) == 0) isErr = 1;
			BCASE INTERP:  if (doInterpret(wd) == 0) isErr = 1;
			break; default: printString("-state?-"); break;
		}
		if (isErr) {
			state = INTERP;
			while (inputFp) { fileClose(inputFp); inputFp = filePop(); }
			return;
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
	blockLoad(1);
}

void baseSys() {
	for (int i = 0; prims[i].name; i++) {
		addWord(prims[i].name)->xt = prims[i].op;
	}

	parseF(": version   #%d ;", VERSION);
	parseF(": (jmp)     #%d ;", JMP);
	parseF(": (jmpz)    #%d ;", JMPZ);
	parseF(": (njmpz)   #%d ;", NJMPZ);
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

	parseF(": code-sz  #%d ;", CODE_SZ);
	parseF(": vars-sz  #%d ;", VARS_SZ);
	parseF(": dict-sz  #%d ;", DICT_SZ);
    parseF(": block-sz #%d ;", BLOCK_SZ);
	parseF(": cell     #%d ;", CELL_SZ);
	sys_load();
}

void Init() {
	fill((byte*)&code[0], (CODE_SZ+1) * sizeof(ushort), 0);
	fill(&vars[0], (VARS_SZ+1), 0);
	fill(&dict[0], (DICT_SZ+1), 0);
	for (int t=0; t<VARS_SZ; t++) { vars[t]=0; }
	for (int t=0; t<DICT_SZ; t++) { dict[t]=0; }
	sp = rsp = lsp = aSp = state = 0;
	last = DICT_SZ;
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
	initBlocks();
	if (argc>1) {
		// load init block first (if necessary)
		cell tmp = fileOpen(argv[1]-1, " rt");
		if (tmp && inputFp) { filePush(tmp); }
		else { inputFp = tmp; }
	}
	while (1) { REP(); };
    flushBlocks();
	return 0;
}

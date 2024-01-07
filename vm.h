// vm - a stack-based VM

#ifndef __VM_H__

#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <math.h>

#ifndef STK_SZ
#define STK_SZ     64
#define LSTK_SZ    30
#define REGS_SZ   100
#define NAME_LEN   21
#endif

#ifndef CODE_SZ
#define CODE_SZ    96*1024
#define VARS_SZ  1024*1024
#endif

typedef uint8_t  byte;
#ifdef _M64_
typedef int64_t  cell_t;
typedef uint64_t ucell_t;
typedef double   flt_t;
#else
typedef long          cell_t;
typedef unsigned long ucell_t;
typedef float         flt_t;
#endif

typedef union { cell_t i; flt_t f; char *c; } se_t;
typedef struct { cell_t sp; se_t stk[STK_SZ+1]; } stk_t;
typedef struct { cell_t xt; byte f; byte len; char name[NAME_LEN+1]; } dict_t;

extern stk_t ds, rs;
extern cell_t lstk[LSTK_SZ+1], lsp, output_fp;
extern cell_t inputStk[10], fileSp, input_fp, output_fp;
extern cell_t state, base, reg[REGS_SZ], reg_base, t1, n1;
extern char code[CODE_SZ], vars[VARS_SZ], tib[256], WD[32];
extern char *here, *vhere, *in, *y;
extern dict_t tempWords[10], *last;

enum {
    STOP = 0, LIT1, LIT, EXIT, CALL, JMP, JMPZ, JMPNZ,
    STORE, CSTORE, FETCH, CFETCH, DUP, SWAP, OVER, DROP,
    ADD, MULT, SLMOD, SUB, INC, DEC, LT, EQ, GT, EQ0,
    RTO, RFETCH, RFROM, DO, LOOP, LOOP2, INDEX,
    COM, AND, OR, XOR, TYPE, ZTYPE,
    REG_I, REG_D, REG_R, REG_RD, REG_RI, REG_S,
    REG_NEW, REG_FREE,
    SYS_OPS, STR_OPS, FLT_OPS
};

// NB: these skip #3 (EXIT), so they can be marked as INLINE
enum { // System opcodes
    INLINE=0, IMMEDIATE, DOT, ITOA=4, ATOI,
    COLONDEF, ENDWORD, CREATE, FIND, WORD, TIMER,
    CCOMMA, COMMA, KEY, QKEY, EMIT, QTYPE
};

enum { // String opcodes
    TRUNC=0, LCASE, UCASE, STRCPY=4, STRCAT, STRCATC, STRLEN, STREQ, STREQI, LTRIM, RTRIM
};

enum { // Floating point opcdes
    FADD=0, FSUB, FMUL, FDIV=4, FEQ, FLT, FGT, F2I, I2F, FDOT,
    SQRT, TANH
};

enum { STOP_LOAD = 99, ALL_DONE = 999, VERSION = 99 };

#define BTW(a,b,c)    ((b<=a) && (a<=c))
#define CELL_SZ       sizeof(cell_t)
#define CpAt(x)       (char*)Fetch((char*)x)
#define ToCP(x)       (char*)(x)
#define ClearTib      fill(tib, 0, sizeof(tib))
#define DSP           ds.sp
#define TOS           (ds.stk[DSP].i)
#define NOS           (ds.stk[DSP-1].i)
#define FTOS          (ds.stk[DSP].f)
#define FNOS          (ds.stk[DSP-1].f)
#define CTOS          (ds.stk[DSP].c)
#define RSP           rs.sp
#define RTOS          (rs.stk[RSP].c)
#define L0            lstk[lsp]
#define L1            lstk[lsp-1]
#define L2            lstk[lsp-2]
#define IS_IMMEDIATE  1
#define IS_INLINE     2
#define NEXT          goto next
#define NCASE         NEXT; case
#define RCASE         return pc; case
#define BCASE         break; case

// These are defined in vm.c
extern void push(cell_t x);
extern cell_t pop();
extern char *cpop();
extern void fpush(flt_t x);
extern flt_t fpop(); 
extern void rpush(char *x); 
extern char *rpop(); 
extern void CComma(cell_t x); 
extern void Comma(cell_t x); 
extern void fill(char *d, char val, int num); 
extern char *strEnd(char *s); 
extern void strCat(char *d, const char *s); 
extern void strCatC(char *d, const char c); 
extern void strCpy(char *d, const char *s); 
extern int strLen(const char *d); 
extern char *lTrim(char *d); 
extern char *rTrim(char *d);
extern int lower(int x); 
extern int upper(int x);
extern int min(int a, int b);
extern int max(int a, int b);

extern int strEqI(const char *s, const char *d);
extern int strEq(const char *d, const char *s);
extern void printStringF(const char *fmt, ...);
extern char *iToA(cell_t N, int b);
extern int isTempWord(const char *w);
extern char isRegOp(const char *w);
extern int nextWord(char *wd);
extern void doDefine(char *wd);
extern int doFind(const char *nm);
extern int isBase10(const char *wd);
extern int isNum(const char *wd);
extern void doType(const char *str);
extern char *doStringOp(char *pc);
extern char *doSysOp(char *pc);
extern char *doFloatOp(char *pc);
extern void Run(char *pc);
extern int doReg(const char *w);
extern void vmInit();

// These are functions vm.c needs to be defined
extern void Store(const char *addr, cell_t val);
extern cell_t Fetch(const char *addr);
extern void printString(const char *str);
extern void printChar(char c);
extern char *doUser(char *pc, char ir);
extern int key();
extern int qKey();
extern cell_t sysTime();

#endif // __VM_H__

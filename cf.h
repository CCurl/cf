#ifndef __CF_H__
#define __CF_H__

#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#ifdef _MSC_VER
#define IS_WINDOWS 1
#define IS_PC      1
#endif

#ifdef IS_LINUX
#define IS_PC      1
#endif

#define RED      1
#define GREEN    2
#define ORANGE   3
#define BLUE     4
#define PURPLE   5
#define CYAN     6
#define WHITE    7

#define DEFINE   RED
#define COMPILE  GREEN
#define INTERP   ORANGE
#define MACRO    BLUE
#define XXXXX0   PURPLE
#define XXXXX1   CYAN
#define COMMENT  WHITE

#define VERSION       240605

#define CODE_SZ       0xFFFF
#define VARS_SZ       0xFFFF
#define DICT_SZ       0xFFFF
#define STK_SZ            63
#define RSTK_SZ           63
#define LSTK_SZ           60
#define FSTK_SZ           10
#define BLOCK_SZ        2500
#define NUM_BLOCKS       400
#define btwi(n,l,h)   (((l)<=n) && (n<=(h)))
#define BCASE         break; case

#if __LONG_MAX__ > __INT32_MAX__
#define CELL_SZ   8
#define FLT_T     double
#define addrFmt ": %s $%llx ;"
#else
#define CELL_SZ   4
#define FLT_T     float
#define addrFmt ": %s $%lx ;"
#endif

typedef long cell;
typedef long cell_t;
typedef unsigned long ucell;
typedef unsigned short ushort;
typedef unsigned char byte;
typedef union { FLT_T f; cell i; } SE_T;
typedef struct { ushort xt; byte sz, fl, lx, ln; char nm[32]; } DE_T;

// These are defined by c4.c
extern void Init();
extern void REP();
extern int  strLen(const char *s);
extern int  strEq(const char *d, const char *s);
extern int  strEqI(const char *d, const char *s);
extern void strCpy(char *d, const char *s);
extern void fill(byte *addr, cell num, byte ch);
extern void printStringF(const char* fmt, ...);
extern void Color(int c, int bg);

// cf.c needs these
extern int  key();
extern int  qKey();
extern void printString(const char *str);
extern void printChar(const char ch);
extern void ttyMode(int isRaw);
extern cell inputFp, outputFp;
extern void fileInit();
extern void filePush(cell fh);
extern cell filePop();
extern cell fileOpen(char *name, char *mode);
extern void fileClose(cell fh);
extern cell fileRead(byte *buf, int sz, cell fh);
extern cell fileWrite(byte *buf, int sz, cell fh);
extern int  fileGets(char *buf, int sz, cell fh);
extern void fileLoad(char *name);
extern void blockLoad(int blk);
extern void doEditor(int blk);

extern void  initBlocks();
extern byte *blockData(int blk);
extern void  blockDirty(int blk, int val);
extern int   blockDirtyQ(int blk);
extern void  blockReload(int blk);
extern void  flushBlocks();

#endif

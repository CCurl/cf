#ifndef __CF_H__
#define __CF_H__

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
// #include "block.h"

#ifdef _MSC_VER
#include <conio.h>
#endif

#ifdef LINUX
#include <sys/time.h>
#include <unistd.h>
#include <termios.h>
#endif

#define RED      1
#define GREEN    2
#define ORANGE   3
#define BLUE     4 // HARD TO SEE!
#define PURPLE   5
#define CYAN     6
#define WHITE    7

#define DEFINE   RED
#define COMPILE  GREEN
#define INTERP   ORANGE
#define XXXXX2   BLUE
#define MLMODE   PURPLE
#define XXXXX1   CYAN
#define COMMENT  WHITE

#define VERSION       240602
#define CODE_SZ       0xFFFF
#define VARS_SZ       0xFFFF
#define DICT_SZ       0xFFFF
#define STK_SZ            63
#define RSTK_SZ           63
#define LSTK_SZ           60
#define FSTK_SZ           10
#define btwi(n,l,h)   ((l<=n) && (n<=h))

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
extern int strLen(const char *s);

// c4.c needs these
extern cell inputFp, outputFp;
extern void fileInit();
extern void filePush(cell fh);
extern cell filePop();
extern cell fileOpen(char *name, char *mode);
extern void fileClose(cell fh);
extern cell fileRead(char *buf, int sz, cell fh);
extern cell fileWrite(char *buf, int sz, cell fh);
extern int  fileGets(char *buf, int sz, cell fh);
extern void fileLoad(char *name);
extern void blockLoad(int blk);

#endif

// A ColorForth inspired system, MIT license

#ifndef __CF_H__

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#define IS_WINDOWS 1
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#define VERSION     20241017

#define MEM_SZ       4*(1024*1024)
#define NAME_MAX          25
#define STK_SZ            63
#define LSTK_SZ           60
#define TSTK_SZ           63
#define btwi(n,l,h)   ((l<=n) && (n<=h))

#if INTPTR_MAX > INT32_MAX
    #define CELL_T    int64_t
    #define UCELL_T   uint64_t
    #define CELL_SZ   8
    #define FLT_T     double
    #define addrFmt ": %s $%llx ;"
#else
    #define CELL_T    int32_t
    #define UCELL_T   uint32_t
    #define CELL_SZ   4
    #define FLT_T     float
    #define addrFmt ": %s $%lx ;"
#endif

enum { DEFINE=1, COMPILE, INTERP, COMMENT };

typedef CELL_T cell;
typedef UCELL_T ucell;
typedef unsigned short ushort;
typedef unsigned char byte;
typedef struct { cell xt; byte flags, len; char name[NAME_MAX+1]; } DE_T;
typedef struct { byte op; const char* name; byte fl; } PRIM_T;

// These are defined by cf.c
extern void inner(cell start);
extern int  outer(const char *src);
extern void Init();

// cf.c needs these to be defined
extern cell state, outputFp;
extern byte mem[];
extern void zType(const char *str);
extern void emit(const char ch);
extern void ttyMode(int isRaw);
extern int  key();
extern int  qKey();
extern cell timer();
extern void ms(cell sleepForMS);
extern cell fOpen(const char *name, cell mode);
extern void fClose(cell fh);
extern cell fRead(cell buf, cell sz, cell fh);
extern cell fWrite(cell buf, cell sz, cell fh);
extern cell fSeek(cell fh, cell offset);

#endif //  __CF_H__

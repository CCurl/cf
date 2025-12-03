// A ColorForth inspired system, MIT license

#ifndef __CF_H__

#ifdef _MSC_VER
    #define _CRT_SECURE_NO_WARNINGS
    #define IS_WINDOWS 1
    #define BOOT_FILE "\\bin\\cf-boot.fth"
#else
    #define BOOT_FILE "/home/chris/bin/cf-boot.fth"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#define VERSION         20251203

#define MEM_SZ          16*(1024*1024)
#define STK_SZ          63
#define LSTK_SZ         60
#define TSTK_SZ         63

#define btwi(n,l,h)     ((l<=n) && (n<=h))
#define NCASE           goto next; case
#define BCASE           break; case

#if INTPTR_MAX > INT32_MAX
    #define CELL_T      int64_t
    #define UCELL_T     uint64_t
    #define CELL_SZ     8
    #define NUM_BITS    0xE000000000000000
    #define NUM_MASK    0x1FFFFFFFFFFFFFFF
    #define NAME_MAX    21
#else
    #define CELL_T      int32_t
    #define UCELL_T     uint32_t
    #define CELL_SZ     4
    #define NUM_BITS    0xE0000000
    #define NUM_MASK    0x1FFFFFFF
    #define NAME_MAX    25
#endif

enum { DEFINE=1, COMPILE, INTERP, COMMENT };
enum { _IMMED=1, _INLINE=2 };

typedef CELL_T cell;
typedef UCELL_T ucell;
typedef unsigned short ushort;
typedef unsigned char byte;
typedef struct { cell xt; byte flags, len; char name[NAME_MAX+1]; } DE_T;
typedef struct { cell op; const char* name; byte fl; } PRIM_T;

// These are defined by cf.c
extern void cfOuter(const char *src);
extern void cfInit();
extern cell state, outputFp;
extern byte mem[];

// cf.c needs these to be defined
extern void zType(const char *str);
extern void emit(const char ch);
extern void ttyMode(int isRaw);
extern int  key();
extern int  qKey();
extern cell timer();
extern void ms(cell sleepForMS);
extern cell fOpen(cell name, cell mode);
extern void fClose(cell fh);
extern cell fRead(cell buf, cell sz, cell fh);
extern cell fWrite(cell buf, cell sz, cell fh);
extern cell fSeek(cell fh, cell offset);

#endif //  __CF_H__

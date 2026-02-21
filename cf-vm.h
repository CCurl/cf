// A Tachyon inspired system, MIT license, (c) 2025 Chris Curl

#ifndef __CF_H__

#define VERSION         20260221
#ifdef _MSC_VER
    #define _CRT_SECURE_NO_WARNINGS
    #define IS_WINDOWS 1
    #define strEqI(s, d)  (_strcmpi(s, d) == 0)
    #define BIN_DIR "D:\\bin\\"
#else
    #define strEqI(s, d)  (strcasecmp(s, d) == 0)
    #define BIN_DIR "/home/chris/bin/"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#if INTPTR_MAX > INT32_MAX
    #define LIT_MASK      0x4000000000000000
    #define LIT_BITS      0x3FFFFFFFFFFFFFFF
    #define CELL_SZ                8
    #define cell             int64_t
    #define ucell           uint64_t
#else
    #define LIT_MASK      0x40000000
    #define LIT_BITS      0x3FFFFFFF
    #define CELL_SZ                4
    #define cell             int32_t
    #define ucell           uint32_t
#endif

#define byte             uint8_t
#define MEM_SZ         0x1000000
#define STK_SZ                63
#define NAME_LEN              25
#define IMMED               0x80
#define INLINE              0x40
#define btwi(n,l,h)   ((l<=n) && (n<=h))
#define TOS           dstk[dsp]
#define NOS           dstk[dsp-1]
#define L0            lstk[lsp]
#define L1            lstk[lsp-1]
#define L2            lstk[lsp-2]

enum { INTERPRET=0, COMPILE=1, BYE=999 };
typedef struct { ucell xt; byte sz; byte fl; byte ln; char nm[NAME_LEN+1]; } DE_T;
typedef struct { char *name; ucell value; } NVP_T;

// These are defined by cf-vm.c
extern void inner(ucell start);
extern void outer(const char *src);
extern void addLit(const char *name, cell val);
extern void cfInit();
extern int nextWord();
extern DE_T *addToDict(const char *w);
extern void compileNum(cell n);
extern cell state;
extern ucell outputFp;
extern char mem[];

// cf-vm.c needs these to be defined
extern void zType(const char *str);
extern void emit(const char ch);
extern int  key();
extern int  qKey();
extern cell timer();
extern void ms(cell sleepForMS);
extern cell fOpen(cell name, cell mode);
extern void fClose(cell fh);
extern cell fRead(cell buf, cell sz, cell fh);
extern cell fWrite(cell buf, cell sz, cell fh);

#endif //  __CF_H__

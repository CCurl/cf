#ifndef __CF_H__
#define __CF_H__

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#ifdef _MSC_VER
#include <conio.h>
#endif

#ifdef LINUX
#include <sys/time.h>
#include <unistd.h>
#include <termios.h>
#endif

typedef int CELL;
typedef unsigned char byte;

typedef struct {
    byte *xt;
    byte fl;
    byte l;
    char name[18];
} DICT_T;

#define BLACK    30
#define RED      91
#define GREEN    92
#define YELLOW   93
#define BLUE     94
#define PURPLE   95
#define CYAN     96
#define WHITE    97

#define COMMENT  '('
#define COMPILE  '^'
#define DEFINE   ':'
#define IMMED    '['
#define ASM      '~'
#define INPUT    5

#define INLINE     0x01
#define IMMEDIATE  0x02

#define STK_SZ     31
#define USER_SZ    1024*1024
#define VARS_SZ    1024*1024
#define CELL_SZ    sizeof(CELL)
#define DICT_SZ    sizeof(DICT_T)

#define betw(x, y, z) ((y <= x) && (x <= z))

typedef enum {
    STOP=0, CALL, 
    JMP, JMPz, JMPn,
    LIT1, LIT4,
} OPCODE_T;

extern char theBlock[];

extern void GotoXY(int, int);
extern void Color(int, int);
extern int qKey();
extern int key();
extern void Color(int fg, int bg);
extern void cfColor(int md);
extern void printString(const char* s);
extern void printStringF(const char* fmt, ...);
extern void printChar(char c);
extern void doEditor(CELL);
extern void doOuter(char *);

#endif 

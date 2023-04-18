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
    byte l;
    char name[31];
} DICT_T;

#define RED      1
#define GREEN    2
#define YELLOW   3
#define BLUE     4
#define PURPLE   5
#define CYAN     6
#define WHITE    7

#define COMMENT  '('
#define COMPILE  '^'
#define DEFINE   ':'
#define INTERP   '_'
#define ASM      '~'

#define STK_SZ     15
#define USER_SZ    1024*1024
#define CELL_SZ    sizeof(CELL)
#define DICT_SZ    sizeof(DICT_T)

#define betw(x, y, z) ((y <= x) && (x <= z))

extern char theBlock[];

extern void GotoXY(int, int);
extern void Color(int, int);
//extern void push(CELL);
//extern CELL pop();
extern int qKey();
extern int key();
extern void Color(int fg, int bg);
extern void printString(const char* s);
extern void printStringF(const char* fmt, ...);
extern void printChar(char c);
extern void doEditor(CELL);
extern void doOuter(char *);

#endif 

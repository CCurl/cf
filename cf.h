#ifndef __CF_H__
#define __CF_H__

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include "vm.h"
#include "block.h"

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
#define COMMENT  GREEN
#define INLINE   ORANGE
#define XXXXX    BLUE
#define MLMODE   PURPLE
#define COMPILE  CYAN
#define INTERP   WHITE

extern void GotoXY(int, int);
extern void Color(int, int);
extern int qKey();
extern int key();
extern void printString(const char* s);
extern void printStringF(const char* fmt, ...);
extern void printChar(char c);
extern void doEditor(cell_t);
extern void doOuter(const char *src);
extern void initialWords();

#endif

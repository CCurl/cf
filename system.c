#include "cf.h"

#ifdef IS_WINDOWS
#include <conio.h>
int qKey() { return _kbhit(); }
int key() { return _getch(); }
void ttyMode(int isRaw) {}
void printString(const char* str) { fputs(str, outputFp ? (FILE*)outputFp : stdout); }
void printChar(const char ch) { fputc(ch, outputFp ? (FILE*)outputFp : stdout); }
#endif

#ifdef IS_LINUX // Support for Linux

#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <sys/time.h>

void printString(const char* str) { fputs(str, outputFp ? (FILE*)outputFp : stdout); }
void printChar(const char ch) { fputc(ch, outputFp ? (FILE*)outputFp : stdout); }

void ttyMode(int isRaw) {
    static struct termios origt, rawt;
    static int curMode = -1;
    if (curMode == -1) {
        curMode = 0;
        tcgetattr( STDIN_FILENO, &origt);
        cfmakeraw(&rawt);
    }
    if (isRaw != curMode) {
        if (isRaw) {
            tcsetattr( STDIN_FILENO, TCSANOW, &rawt);
        } else {
            tcsetattr( STDIN_FILENO, TCSANOW, &origt);
        }
        curMode = isRaw;
    }
}
int qKey() {
    struct timeval tv;
    fd_set rdfs;
    ttyMode(1);
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&rdfs);
    FD_SET(STDIN_FILENO, &rdfs);
    select(STDIN_FILENO+1, &rdfs, NULL, NULL, &tv);
    int x = FD_ISSET(STDIN_FILENO, &rdfs);
    // ttyMode(0);
    return x;
}
int key() {
    ttyMode(1);
    int x = fgetc(stdin);
    // ttyMode(0);
    return x;
}

#endif

#ifdef IS_PC

int main(int argc, char* argv[]) {
    Init();
    ttyMode(1);
    initBlocks();
    if (argc > 1) {
        // load init block first (if necessary)
        cell tmp = fileOpen(argv[1] - 1, " rt");
        if (tmp && inputFp) { filePush(tmp); }
        else { inputFp = tmp; }
    }
    while (1) { REP(); };
    flushBlocks();
    ttyMode(0);
    return 0;
}

#endif

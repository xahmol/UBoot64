#ifndef CORE__H
#define CORE__H

#include "defines.h"
#include "dir.h"

void errorexit();
void CheckStatus(char* message);
void mid(const char *src, size_t start, size_t length, char *dst, size_t dstlen);
void cspaces(unsigned char number);
char* pathconcat();
char getkey(BYTE mask);
int cmd(const BYTE device, const char *cmd);
void execute(char * prg, BYTE device, BYTE boot, char * command);
int textInput(const BYTE xpos, const BYTE ypos, char *str, const BYTE size);
void clearArea(const BYTE xpos, const BYTE ypos, const BYTE xsize, const BYTE ysize);
void initScreen(const BYTE border, const BYTE bg, const BYTE text);
void exitScreen(void);

/// if you change this enum, also change the "drivetype" array in ops.c
enum drive_e {NONE=0, PI1541, D1540, D1541, D1551, D1570, D1571, D1581, D1001, D2031, D8040, SD2IEC, CMD, VICE, U64, LAST_DRIVE_E};

extern BYTE devicetype[];
extern const char* drivetype[];
extern char linebuffer[81];
extern BYTE device;
extern char linebuffer2[];

#endif

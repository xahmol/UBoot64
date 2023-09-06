/* -*- c-basic-offset: 2; tab-width: 2; indent-tabs-mode: nil -*-
 * vi: set shiftwidth=2 tabstop=2 expandtab:
 * :indentSize=2:tabSize=2:noTabs=true:
 */
/** @file
 * \date 10.01.2009
 * \author bader
 *
 * DraCopy (dc*) is a simple copy program.
 * DraBrowse (db*) is a simple file browser.
 *
 * Since both programs make use of kernal routines they shall
 * be able to work with most file oriented IEC devices.
 *
 * Created 2009 by Sascha Bader
 *
 * The code can be used freely as long as you retain
 * a notice describing original source and author.
 *
 * THE PROGRAMS ARE DISTRIBUTED IN THE HOPE THAT THEY WILL BE USEFUL,
 * BUT WITHOUT ANY WARRANTY. USE THEM AT YOUR OWN RISK!
 *
 * https://github.com/doj/dracopy
 */

#ifndef OPS__H
#define OPS__H

#include "defines.h"
#include "dir.h"

void CheckStatus(char* message);
void mid(const char *src, size_t start, size_t length, char *dst, size_t dstlen);
void cspaces(unsigned char number);
char* pathconcat();
char getkey(BYTE mask);
void std_write(char * filename);
void std_read(char * filename);
void writeconfigfile(char* filename);
void readconfigfile(char* filename);
void headertext(char* subtitle);
int cmd(const BYTE device, const char *cmd);
void execute(char * prg, BYTE device, BYTE boot, char * command);
void updateScreen();
void updateMenu(void);
void showDir();
void mainLoopBrowse(void);
void clrDir();
void refreshDir(const BYTE sorted);
void printDir();
void printElementPriv(const BYTE xpos, const BYTE ypos);
const char* fileTypeToStr(BYTE ft);
BYTE dosCommand(const BYTE lfn, const BYTE drive, const BYTE sec_addr, const char *cmd);
void CheckMounttype(const char *dirname);
int changeDir(const BYTE device, const char *dirname, const BYTE sorted);
void drawDirFrame();
const char* getDeviceType(const BYTE device);
int textInput(const BYTE xpos, const BYTE ypos, char *str, const BYTE size);

/// if you change this enum, also change the "drivetype" array in ops.c
enum drive_e {NONE=0, PI1541, D1540, D1541, D1551, D1570, D1571, D1581, D1001, D2031, D8040, SD2IEC, CMD, VICE, U64, LAST_DRIVE_E};

extern BYTE devicetype[];
extern const char* drivetype[];
extern char linebuffer[81];
extern BYTE device;
extern char linebuffer2[];

#endif

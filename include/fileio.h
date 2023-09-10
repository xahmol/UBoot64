#ifndef FILEIO__H
#define FILEIO__H

#include "defines.h"

void FreeSlotMemory();
void std_write(char * filename,unsigned char verbose);
void std_read(char * filename,unsigned char verbose);
void writeconfigfile(char* filename);
void readconfigfile(char* filename);
void headertext(char* subtitle);
int cmd(const BYTE device, const char *cmd);
void execute(char * prg, BYTE device, BYTE boot, char * command);

#endif
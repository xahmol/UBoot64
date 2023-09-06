#ifndef CORE_H_
#define CORE_H_

void CheckStatus(char* message);
void mid(const char *src, size_t start, size_t length, char *dst, size_t dstlen);
void cspaces(unsigned char number);
char* pathconcat();
char getkey(BYTE mask);

#endif
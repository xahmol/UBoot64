#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

#include <c64.h>
#include "magicdesk.h"

//Demo screen parts:
void blot ();			//Draws an ink blot on the screen.
void colorblot ();		//Same, but colored.
void streamtop ();		//Fire curtain from top.
#define numparts 3
#pragma code-name	("CODE2")
#pragma rodata-name	("RODATA2")
static const unsigned char partid[]={1,2,3};
void main()
{
	unsigned char c;
	bgcolor (1); bordercolor (4); //textcolor (5);
	//clrscr();

	puts	("\f\x90\x93\x12\x89   Screen Demo of Final Cart. 3 Format  \x92"
		"   by \x9aJoseph Rose\x99, a.k.a. \x96\x12Harry Potter\x99\n"
		"----------------------------------------\x90"

		"Choose your demo:\n\n"
		"1.  Ink Blot\n"
		"2.  \x1C""C\x81o\x9El\x1Eo\x1Fr\x90 Ink Blot\n"
		"3.  \x1C""Fire\x90 Curtain from Top\n"
		"4.  Exit"
		);
	while (1) {
		c=cgetc()-'1';
		if(c==3) { bankout(); }
		if (c<numparts) bankrun (partid[c]);
	}
}


#pragma rodata-name	("RODATA")
const struct codetable codetable [] =
{
	{&main,		1},
	{&blot,		2},
	{&colorblot,	2},
	{&streamtop,	3}
};

#pragma rodata-name	("RODATA2")

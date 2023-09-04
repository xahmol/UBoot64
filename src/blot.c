#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <conio.h>

#include <c64.h>

#include "magicdesk.h"

#pragma code-name	("CODE3")
#pragma rodata-name	("RODATA3")

static const unsigned char blotc [4]={0,1,2,5};
void blot ()
{
	static unsigned i;
	_randomize ();

	bordercolor (3); bgcolor(1); textcolor (0);
	clrscr ();
	for (i=0; i<20*40; ++i) {revers (rand()&1); cputc(' ');}
	revers(0);
	cputsxy (0,24,"Press any key to continue...");
	//gotoxy (0,24); cputs ("Press any key to continue...");
	while (!kbhit());
	cgetc();
	bankrun (0);
}


void colorblot ()
{
	static unsigned i;
	_randomize ();

	bordercolor (3); bgcolor(1); textcolor (0);
	clrscr (); revers(1);
	for (i=0; i<20*40; ++i) {textcolor (blotc[rand()&3]); cputc(' ');}
	revers(0);
	textcolor (0);
	cputsxy (0,24,"Press any key to continue...");
	while (!kbhit());
	cgetc();
	bankrun (0);
}


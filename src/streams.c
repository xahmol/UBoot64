#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <c64.h>
#include "magicdesk.h"

#pragma code-name	("CODE4")
#pragma rodata-name	("RODATA4")

static char toppos [40], leftpos [20];

static void setup ()
{
	/*static unsigned i;*/ static unsigned char j;
	bordercolor (8); bgcolor (0); textcolor (0); clrscr();
	revers(1);
	for (j=0; j<24;++j) cclearxy (0, j, 40);
	//revers(0);
	_randomize();
}

void streamtop ()
{
	static unsigned char c, d;
	static unsigned i;
	bzero (&toppos, 40);
	setup();
	while (!kbhit()) {
		c=rand()%40;
		d=++toppos[c]; if (d>23) --toppos[c];
		//if (d==24) break;
		textcolor (9); cputcxy (c,d,' ');
		if (d<1) goto skip0;
		textcolor (2); cputcxy (c,d-1,' ');
		if (d<2) goto skip0;
		textcolor (8); cputcxy (c,d-2,' ');
		if (d<3) goto skip0;
		textcolor (7); cputcxy (c,d-3,' ');
		if (d<4) goto skip0;
		textcolor (1); cputcxy (c,d-4,' ');
skip0:
		;//for (i=0x800; i>0; --i);
	}
	textcolor (0); revers(0);
	//cputsxy (0,24, "Press any key to continue...");
	//cgetc();
	bankrun (0);
}

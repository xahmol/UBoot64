// UBoot64:
// Boot menu for C64 Ultimate enabled devices
// Written in 2023 by Xander Mol
// https://github.com/xahmol/UBoot64
// https://www.idreamtin8bits.com/
//
// Inspired by and code used from DraBrowse:
// DraBrowse (db*) is a simple file browser.
// Originally created 2009 by Sascha Bader.
// Used version adapted by Dirk Jagdmann (doj)
// https://github.com/doj/dracopy
//
// Most of code and functionality ported from:
// DMBoot 128:
// Device Manager Boot Menu for the Commodore 128
// Written in 2020-2023 by Xander Mol
// https://github.com/xahmol/DMBoot
//
// Additionally uses code from:
// - Ultimate 64/II+ Command Library
//   Scott Hutter, Francesco Sblendorio
//   https://github.com/xlar54/ultimateii-dos-lib
// - GenCart64 - cc65 Library for C64 cartridges
//   Joseph Rose, a.k.a. Harry Potter
//   https://sourceforge.net/projects/cc65extra/files/memory%20cfgs/
//   Used for inspiration for building C64 cartridge image with CC65
// - Sidekick64 by frntc
//   https://github.com/frntc/Sidekick64/blob/master/Source/Firmware/C64Side/cart.a
//   Used as inspiration for cartridge init and exit code
// - UUC based on Final Cartridge III by Bart van Leeuwen / bvl1999
//   https://github.com/bvl1999/final_cartridge/blob/master/core/init.s
//   Used for inspiration for building C64 cartridge image with CC65
// - ntp2ultimate by MaxPlap
//   https://github.com/MaxPlap/ntp2ultimate
//   Time via NTP code
// - EPOCH-to-time-date-converter by sidsingh78
//   https://github.com/sidsingh78/EPOCH-to-time-date-converter/blob/master/epoch_conv.c
// - petcom version 1.00 by Craig Bruce, 18-May-1995
//   Convert from PETSCII to ASCII, or vice-versa.
//   https://codebase64.org/doku.php?id=source_conversion
//
// Requires and made possible by the Ultimate II+ cartridge,
// Created by Gideon Zweijtzer
// https://ultimate64.com/
//
// The code can be used freely as long as you retain
// a notice describing original source and author.
//
// THE PROGRAMS ARE DISTRIBUTED IN THE HOPE THAT THEY WILL BE USEFUL,
// BUT WITHOUT ANY WARRANTY. USE THEM AT YOUR OWN RISK!

//Includes
#include <stdio.h>
#include <string.h>
#include <peekpoke.h>
#include <cbm.h>
#include <conio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <device.h>
#include <c64.h>
#include <time.h>
#include "defines.h"
#include "version.h"
#include "core.h"
#include "fileio.h"
#include "slotmenu.h"
#include "u-time.h"
#include "ultimate_common_lib.h"
#include "ultimate_dos_lib.h"
#include "ultimate_time_lib.h"
#include "ultimate_network_lib.h"
#include "fc3.h"

#pragma code-name	("CODE2");
#pragma rodata-name	("RODATA2");

// Entrypoints in other banks prototypes
void mainLoopBrowse(void);

// Global variables
BYTE SCREENW;
BYTE DIRW;
BYTE MENUX;
char path[8][20];
char pathfile[20];
BYTE pathdevice;
BYTE pathrunboot;
BYTE depth = 0;
BYTE trace = 0;
BYTE comma1 = 1;
BYTE commandflag = 0;
BYTE reuflag = 0;
BYTE addmountflag = 0;
BYTE runmountflag = 0;
BYTE mountflag = 0;

struct SlotStruct* Slot;
struct SlotStruct* FirstSlot;
struct SlotStruct* BufferSlot;
long secondsfromutc = 7200; 
unsigned char timeonflag = 1;
char host[80] = "pool.ntp.org";
char imagename[20];
char reufilepath[100];
char imageaname[20] = "";
char imageapath[100] = "";
unsigned char imageaid = 0;
char imagebname[20] = "";
char imagebpath[100] = "";
unsigned char imagebid = 0;
unsigned char reusize = 2;
char* reusizelist[8] = { "128 KB","256 KB","512 KB","1 MB","2 MB","4 MB","8 MB","16 MB"};
unsigned char utilbuffer[86];
char configfilename[11] = "dmbcfg.cfg";
char slotfilename[11] = "dmbslt.cfg";
unsigned char configversion = CFGVERSION;
unsigned char menuselect;
unsigned char fb_selection_made = 0;
unsigned char fb_uci_mode;
unsigned char inside_mount;

//Main program
void main() {

    SCREENW = 40;  //Set flag for 40 column
    DIRW = 25;
    MENUX = 25;

    if(!fb_selection_made) {
        if(!uii_detect()) {
            cputs("No Ultimate Command Interface enabled.");
            printf("\n\rPress key to exit.\n\r");
            cgetc();
            bankout();
        } else { printf("Ultimate Command Interface detected.\n\r"); }

        uii_change_dir(UCI_CFG_LOC);
	    printf("\nDir changed\nStatus: %s", uii_status);

	    readconfigfile(configfilename);

        // Load slot config
        FirstSlot = calloc(19,SLOTSIZE);

        // Abort if insufficient memory
        if(!FirstSlot) {
            printf("\n\rOut of memory.\n\r");
            errorexit();
        }
        std_read(slotfilename,1); // Read config file

        // Set time from NTP server
        time_main();
    } else {
        // Restore slots in memory returning from filebrowser
        std_read(slotfilename,1);
        if(fb_selection_made==1) { pickmenuslot(); }
    }

    // Disable sprite logo
    POKE(VIC_SPR_ENA,0);                // Disable all sprites via VIC register

    // Init screen and menu
    initScreen(DC_COLOR_BORDER, DC_COLOR_BG, DC_COLOR_TEXT);

    do
    {
        mainmenu();
//
    //    if((menuselect>47 && menuselect<58) || (menuselect>64 && menuselect<91))
    //    // Menuslots 0-9, a-z
    //    {
    //        runbootfrommenu(keytomenuslot(menuselect));
    //    }
//
        switch (menuselect)
        {
        case CH_F1:
            // Filebrowser
            bankrun(1);  // Jump to bank of filebrowser and start entry point
            break;

        case CH_F2:
            // Information and credits
            information();
            break;
        
        case CH_F3:
            // Edit / re-order and delete menuslots
            editmenuoptions();
            break;
                
        case CH_F5:
            edittimeconfig();
            break;
        
        default:
            break;
        }
    } while (menuselect != CH_F7);

    free(FirstSlot);
    bankout();
}

#pragma rodata-name	("RODATA");

const struct codetable codetable [] =
{
	{&main,		1},
	{&mainLoopBrowse,		2}
};
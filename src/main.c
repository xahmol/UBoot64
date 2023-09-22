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
//   Also help with ideas, feednack, error solving, testing and code snippets
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
BYTE demomode = 0;
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
char iec_devices[23];

//Main program
void main() {
    unsigned char clock,x;

    SCREENW = 40;  //Set flag for 40 column
    DIRW = 25;
    MENUX = 25;

    if(!fb_selection_made) {
        
        // Is Ultimate Command Interface detected? If no, abort
        if(!uii_detect()) {
            cputs("No Ultimate Command Interface enabled.");
            printf("\n\rPress key to exit.\n\r");
            cgetc();
            bankout();
        } else { printf("Ultimate Command Interface detected.\n\r"); }

        // Feedback on UCI DOS version
        uii_identify();
        printf("%s",uii_data);

        // Wait for USB to be present by looping till dirchange to root successful
        // Times out on 5 secs
        cia_seconds = 0;
        cia_tensofsec = 0;
        clock = cia_seconds;
        do
        {
            uii_change_dir(UCI_CFG_LOC);
        } while (!uii_success() || clock>4);
        if(!uii_success()) { printf("\n\rUSB storage not found.\n\r"); }

        // Read config file
	    readconfigfile(configfilename);

        // Allocate slot memory
        FirstSlot = calloc(19,SLOTSIZE);
        // Abort if insufficient memory
        if(!FirstSlot) {
            printf("\n\rOut of memory.\n\r");
            errorexit();
        }

        // Read config file
        std_read(slotfilename,1);

        // Read (and print feedback of) drive configuration
        if(!uii_parse_deviceinfo()) {
            printf("Getting device info fails.");
            errorexit();
        }
        printf("\n\n\rRecognised Ultimate devices:\n\r");
        if(uii_devinfo[0].exist) {
            printf("Drive A: ID %2d Pow %s, %s\n\r",uii_devinfo[0].id,(uii_devinfo[0].power)?"On":"Off",uii_device_tyoe(uii_devinfo[0].type));
        }
        if(uii_devinfo[1].exist) {
            printf("Drive B: ID %2d Pow %s, %s\n\r",uii_devinfo[1].id,(uii_devinfo[1].power)?"On":"Off",uii_device_tyoe(uii_devinfo[1].type));
        }
        if(uii_devinfo[2].exist) {
            printf("SoftIEC: ID %2d Pow %s\n\r",uii_devinfo[2].id,(uii_devinfo[2].power)?"On":"Off");
        }
        if(uii_devinfo[3].exist) {
            printf("Printer: ID %2d Pow %s\n\r",uii_devinfo[3].id,(uii_devinfo[3].power)?"On":"Off");
        }
        printf("IDs needing manual power switching: %s\n\r",(CheckActiveIECdevices())?"Yes":"No");
        printf("Active IEC IDs: ");
        for(x=0;x<23;x++) {
            if(iec_devices[x]) {
                printf("%02d ",(x==22)?4:x+8);
            }
        }
        printf("\n\r");

        // Set time from NTP server
        time_main();

        // Uncomment to pause on boot status feedback for debug
        //cgetc();
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

        if((menuselect>47 && menuselect<58) || (menuselect>64 && menuselect<91))
        // Menuslots 0-9, a-z
        {
            runbootfrommenu(keytomenuslot(menuselect));
        }

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
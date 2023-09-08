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
#include "ops.h"
#include "u-time.h"
//#include "screen.h"
//#include "version.h"
//#include "base.h"
//#include "bootmenu.h"
//#include "utils.h"
#include "ultimate_common_lib.h"
#include "ultimate_dos_lib.h"
#include "ultimate_time_lib.h"
#include "ultimate_network_lib.h"
#include "fc3.h"

#pragma code-name	("CODE2");
#pragma rodata-name	("RODATA2");

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
BYTE forceeight = 0;
BYTE fastflag = 0;
BYTE commandflag = 0;
BYTE reuflag = 0;
BYTE addmountflag = 0;
BYTE runmountflag = 0;
BYTE mountflag = 0;

struct SlotStruct Slot;
char newmenuname[18][21];
unsigned int newmenuoldslot[18];
BYTE bootdevice;
long secondsfromutc = 0; 
unsigned char timeonflag = 1;
char host[80] = "pool.ntp.org";
char imagename[20] = "default.reu";
char reufilepath[60] = "/usb*/";
char imageaname[20] = "";
char imageapath[60] = "";
unsigned char imageaid = 0;
char imagebname[20] = "";
char imagebpath[60] = "";
unsigned char imagebid = 0;
unsigned char reusize = 2;
char* reusizelist[8] = { "128 KB","256 KB","512 KB","1 MB","2 MB","4 MB","8 MB","16 MB"};
unsigned char utilbuffer[328];
char configfilename[11] = "dmbcfg.cfg";
unsigned int dm_apiversion = 0;
unsigned char configversion = CFGVERSION;
unsigned int slotaddress_start = 0;

// Get NTP time functions
unsigned char CheckStatusTime()
{
    // Function to check UII+ status

    if (uii_status[0] != '0' || uii_status[1] != '0') {
        printf("\nStatus: %s Data:%s", uii_status, uii_data);
        return 1;
    }
    return 0;
}

void get_ntp_time()
{
    // Function to get time from NTP server and set UII+ time with this

    struct tm *datetime;
    extern struct _timezone _tz;
    unsigned char attempt = 1;
    unsigned char clock;
    char settime[6];
    unsigned char fullcmd[] = { 0x00, NET_CMD_SOCKET_WRITE, 0x00, \
                               0x1b, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, \
                               0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, \
                               0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    unsigned char socket = 0;
    time_t t;
    char res[32];

    printf("\nUpdating UII+ time from NTP Server.");
    uii_get_time();
    printf("\nUltimate datetime: %s", uii_data);

    printf("\nConnecting to: %s", host);
	socket = uii_udpconnect(host, 123); //https://github.com/markusC64/1541ultimate2/blob/master/software/io/network/network_target.cc
    if(CheckStatusTime()) {
        uii_socketclose(socket);
        return;
    }

    printf("\nSending NTP request");
	fullcmd[2] = socket;
    uii_settarget(TARGET_NETWORK);
    uii_sendcommand(fullcmd, 51);//3 + sizeof( ntp_packet ));
	uii_readstatus();
	uii_accept();
    if(CheckStatusTime()) {
        uii_socketclose(socket);
        return;
    }

    // Do maximum of 4 attempts at receiving data
    do
    {
        // Add delay of a second to avoid time to wait on response being too short
        for(clock=0;clock<255;clock++) { ; }

        // Print attempt number
        printf("\nReading result attempt %d",attempt);

        // Try to read incoming data        
        uii_socketread(socket, 50);// 2 + sizeof( ntp_packet ));

        // If data received, end loop. Else do new attempt till counter = 5
        if(uii_success()) { 
            attempt = 5;
        } else {
            attempt++;
        }

    } while (attempt<5);
        
    if(CheckStatusTime()) {
        uii_socketclose(socket);
        return;
    }

    // Convert time received to UCI format
    t = uii_data[37] | (((unsigned long)uii_data[36])<<8)| (((unsigned long)uii_data[35])<<16)| (((unsigned long)uii_data[34])<<24);
    t -= NTP_TIMESTAMP_DELTA;
    
    // Close socket
    uii_socketclose(socket);

    // Print time received and parse to UII+ format
    printf("\nUnix epoch %lu", t);
    _tz.timezone = secondsfromutc;
    datetime = localtime(&t);
    if (strftime(res, sizeof(res), "%F %H:%M:%S", datetime) == 0){
        printf("\nError cannot parse date");
        return;
    }
    printf("\nNTP datetime: %s", res);

    // Set UII+ RTC clock
    settime[0]=datetime->tm_year;
    settime[1]=datetime->tm_mon + 1;
    settime[2]=datetime->tm_mday;
    settime[3]=datetime->tm_hour;
    settime[4]=datetime->tm_min;
    settime[5]=datetime->tm_sec;
    uii_set_time(settime);
    printf("\nStatus: %s", uii_status);
}

void time_main()
{
    if(timeonflag == 1)
    {
        get_ntp_time();
    }

    // Uncomment for debug
    //cgetc();
}

//Main program
void main() {
    int menuselect;

    SCREENW = 40;  //Set flag for 40 column
    DIRW = 25;
    MENUX = 25;

    if(!uii_detect()) {
        cputs("No Ultimate Command Interface enabled.");
        printf("\n\rPress key to exit.\n\r");
        cgetc();
        bankout();
    } else { 		printf("Ultimate Command Interface detected.\n\r"); }

    uii_change_dir("/usb*/");
	printf("\nDir changed\nStatus: %s", uii_status);

	readconfigfile(configfilename);

    // Load slot config
    std_read("dmbslt.cfg",1); // Read config file

    // Set time from NTP server
    time_main();

    // Init screen and menu
    //initScreen(DC_COLOR_BORDER, DC_COLOR_BG, DC_COLOR_TEXT);

    //do
    //{
    //    menuselect = mainmenu();
//
    //    if((menuselect>47 && menuselect<58) || (menuselect>64 && menuselect<91))
    //    // Menuslots 0-9, a-z
    //    {
    //        runbootfrommenu(keytomenuslot(menuselect));
    //    }
//
    //    switch (menuselect)
    //    {
    //    case CH_F1:
    //        // Filebrowser
    //        loadoverlay("11:dmb-fb");        // Load overlay of file browser routines
    //        mainLoopBrowse();
    //        loadoverlay("11:dmb-menu");      // Load overlay of main DMBoot menu routines
    //        if (trace == 1)
    //        {
    //            pickmenuslot();
    //        }
    //        break;
    //    
    //    case CH_F5:
    //        // Go to C64 mode
    //        commandfrommenu("go 64", 1);
    //        break;
//
    //    case CH_F6:
    //        // GEOS RAM boot
    //        clrscr();
    //        textcolor(DC_COLOR_TEXT);
    //        loadoverlay("11:dmb-geos");      // Load GEOS assembly code
    //        geosboot_main();
    //        break;
//
    //    case CH_F4:
    //        loadoverlay("11:dmb-util");      // Load util routines
    //        config_main();
    //        loadoverlay("11:dmb-menu");      // Load overlay of main DMBoot menu routines
    //        break;
//
    //    case CH_F2:
    //        // Information and credits
    //        loadoverlay("11:dmb-util");      // Load util routines
    //        information();
    //        loadoverlay("11:dmb-menu");      // Load overlay of main DMBoot menu routines
    //        break;
    //    
    //    case CH_F7:
    //        // Edit / re-order and delete menuslots
    //        editmenuoptions();
    //        break;
    //    
    //    default:
    //        break;
    //    }
    //} while (menuselect != CH_F3);
//
    //exitScreen();
    //commandfrommenu("scnclr:new",0);    // Erase memory and clear screen on exit
    cgetc();
    bankout();
}

#pragma rodata-name	("RODATA")
const struct codetable codetable [] =
{
	{&main,		1},
};
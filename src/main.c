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
#include "version.h"
#include "core.h"
#include "fileio.h"
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
unsigned char menuselect;
unsigned char fb_selection_made = 0;

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

// Menu slot functions
//void pickmenuslot()
//{
//    // Routine to pick a slot to store the chosen dir trace path
//    
//    int menuslot;
//    char key,devid,plusmin;
//    BYTE yesno;
//    BYTE selected = 0;
//    char deviceidbuffer[3];
//    char* ptrend;
//    
//    clrscr();
//    headertext("Choose menuslot for chosen start option.");
//    presentmenuslots();
//    gotoxy(0,21);
//    cputs("Choose slot by pressing key: ");
//    do
//    {
//        key = cgetc();
//    } while ((key<48 || key>57) && (key<65 || key>90));  
//    menuslot = keytomenuslot(key);
//    selected = 1;
//    getslotfromem(menuslot);
//    cprintf("%c", key);
//    if ( strlen(Slot.menu) != 0 )
//    {
//        gotoxy(0,22);
//        cprintf("Slot not empty. Edit? Y/N ");
//        yesno = getkey(128);
//        cprintf("%c", yesno);
//        if ( yesno == 78 )
//        {
//            selected = 0;
//        }
//    } else {
//        strcpy(Slot.menu, pathfile);
//    }
//    if ( selected == 1)
//    {
//        clearArea(0,22,80,3);
//        gotoxy(0,23);
//        cputs("Choose name for slot:");
//        textInput(0,24,Slot.menu,20);
//
//        clearArea(0,23,80,2);
//        gotoxy(0,23);
//        if(reuflag || addmountflag) {
//            if(reuflag) {
//
//                cputs("Select REU size (+/-/ENTER):");
//
//                do
//                {
//                  gotoxy(0,24);
//                  cprintf("REU file size: (%i) %s",Slot.reusize,reusizelist[Slot.reusize]);
//                  do
//                  {
//                    plusmin = cgetc();
//                  } while (plusmin != '+' && plusmin != '-' && plusmin != CH_ENTER);
//                  if(plusmin == '+')
//                  {
//                      Slot.reusize++;
//                      if(Slot.reusize > 7) { Slot.reusize = 0; }
//                  }
//                  if(plusmin == '-')
//                  {
//                      if(Slot.reusize == 0) { Slot.reusize = 7; }
//                      else { Slot.reusize--; }       
//                  }
//                } while (plusmin != CH_ENTER);
//                strcpy(Slot.reu_image,imagename);
//                Slot.command = Slot.command | COMMAND_REU;
//            } else {
//                sprintf(deviceidbuffer,"%d",addmountflag==1?Slot.image_a_id:Slot.image_b_id);
//                cputs("Enter drive ID:       ");
//                textInput(0,24,deviceidbuffer,2);
//                devid = (unsigned char)strtol(deviceidbuffer,&ptrend,10);
//                if(addmountflag==1) {
//                    strcpy(Slot.image_a_path, pathconcat());
//                    strcpy(Slot.image_a_file,imageaname);
//                    Slot.image_a_id = devid;
//                    Slot.command = Slot.command | COMMAND_IMGA;
//                } else {
//                    strcpy(Slot.image_b_path, pathconcat());
//                    strcpy(Slot.image_b_file,imagebname);
//                    Slot.image_b_id = devid;
//                    Slot.command = Slot.command | COMMAND_IMGB;
//                }
//            }
//        } else {
//            Slot.device = pathdevice;
//            strcpy(Slot.file, pathfile);
//            if(runmountflag) {
//                strcpy(Slot.path, "");
//            } else {
//                strcpy(Slot.path, pathconcat());
//            }
//            Slot.runboot = pathrunboot;
//
//            if ( devicetype[pathdevice] != U64 && forceeight)
//            {
//                Slot.runboot = pathrunboot - EXEC_FRC8;
//            }
//        }
//
//        putslottoem(menuslot);
//
//        gotoxy(0,24);
//        cputs("Saving. Please wait.          ");
//        std_write("dmbootconf");
//    }
//}

char menuslotkey(int slotnumber)
{
    // Routine to convert numerical slotnumber to key in menu
    // Input: Slotnumber = menu slot number
    // Output: Corresponding 0-9, a-z key

    if(slotnumber<10)
    {
        return slotnumber+48; // Numbers 0-9
    }
    else
    {
        return slotnumber+87; // Letters a-z
    }
}

int keytomenuslot(char keypress)
{
    // Routine to convert keypress to numerical slotnumber
    // Input: keypress = ASCII value of key pressed 0-9 or a-z
    // Output: Corresponding menuslotnumber

    if(keypress>64)
    {
        return keypress - 55;
    }
    else
    {
        return keypress - 48;
    }
}

void mainmenu()
{
    // Draw main boot menu
    // Returns chosen menu option as char key value

    int x;
    int select;
    char key;

    clrscr();
    headertext("Welcome to your Commodore 64.");

    for ( x=0 ; x<18 ; ++x )
    {
        gotoxy(0,x+3);
        memcpy(&Slot,(void*)(slotaddress_start + (x*sizeof(Slot))),sizeof(Slot));
        if ( strlen(Slot.menu) != 0 )
        {
            revers(1);
            textcolor(DMB_COLOR_SELECT);
            cprintf(" %2c ",menuslotkey(x));
            revers(0);
            textcolor(DC_COLOR_TEXT);
            cprintf(" %s",Slot.menu);
        }
    }

    gotoxy(0,21);
    revers(1);
    textcolor(DMB_COLOR_SELECT);
    cputs(" F1 ");
    revers(0);
    textcolor(DC_COLOR_TEXT);
    cputs(" Filebrowser");
   
    gotoxy(20,21);
    revers(1);
    textcolor(DMB_COLOR_SELECT);
    cputs(" F2 ");
    revers(0);
    textcolor(DC_COLOR_TEXT);
    cputs(" Information");

    gotoxy(0,22);
    revers(1);
    textcolor(DMB_COLOR_SELECT);
    cputs(" F3 ");
    revers(0);
    textcolor(DC_COLOR_TEXT);
    cputs(" Quit to BASIC");
    
    gotoxy(20,22);
    revers(1);
    textcolor(DMB_COLOR_SELECT);
    cputs(" F4 ");
    revers(0);
    textcolor(DC_COLOR_TEXT);
    cputs(" NTP time config");

    gotoxy(0,23);
    revers(1);
    textcolor(DMB_COLOR_SELECT);
    cputs(" F7 ");
    revers(0);
    textcolor(DC_COLOR_TEXT);
    cputs(" Edit/Reorder/Delete menu");

    cputsxy(0,24,"Make your choice.");

    select = 0;

    do
    {
        key = cgetc();
        if (key == CH_F1 || key == CH_F2 || key == CH_F3 || key == CH_F4 || key == CH_F5 || key == CH_F6 || key == CH_F7)
        {
            select = 1;
        }
        else
        {
            if((key>47 && key<58) || (key>64 && key<91))    // If keys 0 - 9 or a - z
            {
                memcpy(&Slot,(void*)(slotaddress_start + (keytomenuslot(key)*sizeof(Slot))),sizeof(Slot));
                if(strlen(Slot.menu) != 0)   // Check if menslot is empty
                {
                    select = 1;
                }
            }
        }
    } while (select == 0);
    menuselect = key;    
}

//void mountimage(unsigned char device, char* path, char* image) {
//    uii_change_dir(path);
//    uii_mount_disk(device,image);
//}
//
//void runbootfrommenu(int select)
//{
//    // Function to execute selected boot option choice slot 0-9
//    // Input: select: chosen menuslot 0-9
//
//    getslotfromem(select);
//
//    clrscr();
//    gotoxy(0,0);
//    if(Slot.command & COMMAND_IMGA) {
//        cprintf("%s on ID %d.\n\r",Slot.image_a_file,Slot.image_a_id);
//        mountimage(Slot.image_a_id,Slot.image_a_path,Slot.image_a_file);
//    }
//    if(Slot.command & COMMAND_IMGB) {
//        cprintf("%s on ID %d.\n\r",Slot.image_b_file,Slot.image_b_id);
//        mountimage(Slot.image_b_id,Slot.image_b_path,Slot.image_b_file);
//    }
//    if(Slot.command & COMMAND_REU) {
//        cprintf("REU file %s",Slot.reu_image);
//        uii_change_dir(Slot.image_a_path);
//        uii_open_file(1, Slot.reu_image);
//        uii_load_reu(Slot.reusize);
//        uii_close_file();
//    }
//
//    // Enter correct directory path on correct device number
//    if(Slot.runboot & EXEC_MOUNT) {
//        // Run from mounted disk
//        execute(Slot.file,Slot.image_a_id,Slot.runboot,Slot.cmd);
//    } else {
//        // Run from hyperspeed filesystem
//        cmd(Slot.device,Slot.path);
//        execute(Slot.file,Slot.device,Slot.runboot,Slot.cmd);
//    }
//}
//
//void commandfrommenu(char * command, int confirm)
//{
//    // Function to type specified command and execute by placing chars
//    // in keyboard buffer.
//    // Input:
//    // command: command to be executed
//    // confirm: is confirmation of command needed. 0 is no, 1 is yes.
//
//    // prepare the screen with the basic command to load the next program
//    exitScreen();
//    gotoxy(0,2);
//
//    cprintf("%s",command);
//
//    // put CR in keyboard buffer
//    *((unsigned char *)KBCHARS)=13;
//    if (confirm == 1)  // if confirm is 1 also put 'y'+CR in buffer
//    {
//        *((unsigned char *)KBCHARS+1)=89;  // place 'y'
//        *((unsigned char *)KBCHARS+2)=13;  // place CR
//        *((unsigned char *)KBNUM)=3;
//    }
//    else
//    {
//        *((unsigned char *)KBNUM)=1;
//    }
//
//    // exit DraCopy, which will execute the BASIC LOAD above
//    gotoxy(0,0);
//    exit(0);
//}
//
//void editmenuoptions()
//{
//    // Routine for edit / re-order / delete menu slots
//
//    int changesmade = 0;
//    int select = 0;
//    char key;
//
//    do
//    {
//        clrscr();
//        headertext("Edit/Re-order/Delete menu slots");
//
//        presentmenuslots();
//
//        if(SCREENW==80)
//        {
//            gotoxy(0,21);
//        }
//        else
//        {
//            gotoxy(0,18);
//        }
//        cputs("Choose:");
//
//        if(SCREENW==80)
//        {
//            gotoxy(0,22);
//        }
//        else
//        {
//            gotoxy(0,19);
//        }
//        revers(1);
//        textcolor(DMB_COLOR_SELECT);
//        cprintf(" F1 ");
//        revers(0);
//        textcolor(DC_COLOR_TEXT);
//        cputs(" Edit menuslot name");
//
//        if(SCREENW==80)
//        {
//            gotoxy(40,22);
//        }
//        else
//        {
//            gotoxy(0,20);
//        }
//        revers(1);
//        textcolor(DMB_COLOR_SELECT);
//        cprintf(" F2 ");
//        revers(0);
//        textcolor(DC_COLOR_TEXT);
//        cputs(" Edit userdefined command");
//
//        if(SCREENW==80)
//        {
//            gotoxy(0,23);
//        }
//        else
//        {
//            gotoxy(0,21);
//        }
//        revers(1);
//        textcolor(DMB_COLOR_SELECT);
//        cprintf(" F3 ");
//        revers(0);
//        textcolor(DC_COLOR_TEXT);
//        cputs(" Re-order menuslot");
//
//        if(SCREENW==80)
//        {
//            gotoxy(40,23);
//        }
//        else
//        {
//            gotoxy(0,22);
//        }
//        revers(1);
//        textcolor(DMB_COLOR_SELECT);
//        cprintf(" F5 ");
//        revers(0);
//        textcolor(DC_COLOR_TEXT);
//        cputs(" Delete menuslot");
//
//        if(SCREENW==80)
//        {
//            gotoxy(0,24);
//        }
//        else
//        {
//            gotoxy(0,23);
//        }
//        revers(1);
//        textcolor(DMB_COLOR_SELECT);
//        cprintf(" F7 ");
//        revers(0);
//        textcolor(DC_COLOR_TEXT);
//        cputs(" Quit to main menu");
//
//        select = 0;
//
//        do
//        {
//            key = cgetc();
//            if (key == CH_F1 || key == CH_F2 || key == CH_F3 || key == CH_F5 || key == CH_F7)
//            {
//                select = 1;
//            }
//        } while (select == 0);
//
//        switch (key)
//        {
//        case CH_F5:
//            changesmade = deletemenuslot();
//            break;
//
//        case CH_F1:
//            changesmade = renamemenuslot();
//            break;
//
//        case CH_F2:
//            changesmade = edituserdefinedcommand();
//            break;
//
//        case CH_F3:
//            changesmade = reordermenuslot();
//            break;
//        
//        
//        default:
//            break;
//        }
//
//    } while (key != CH_F7);
//    
//    if (changesmade == 1)
//    {
//        gotoxy(0,24);
//        cputs("Saving. Please wait.          ");
//        std_write("dmbootconf");
//    } 
//}
//
//void presentmenuslots()
//{
//    // Routine to show the present menu slots
//    
//    int x;
//
//    for ( x=0 ; x<36 ; ++x )
//    {
//        if (SCREENW==40 && x>14)
//        {
//            break;
//        }
//        if (x>17)
//        {
//            gotoxy(40,x-15);
//        }
//        else
//        {
//            gotoxy(0,x+3);
//        }
//
//        getslotfromem(x);
//
//        revers(1);
//        textcolor(DMB_COLOR_SELECT);
//        cprintf(" %2c ",menuslotkey(x));
//        revers(0);
//        textcolor(DC_COLOR_TEXT);
//        if ( strlen(Slot.menu) == 0 )
//        {
//            cputs(" <EMPTY>");
//        }
//        else
//        {
//            cprintf(" %s",Slot.menu);
//        }
//    }
//}
//
//int deletemenuslot()
//{
//    // Routine to delete a chosen menu slot
//    // Returns 1 if something has been deleted, else 0
//
//    int menuslot;
//    int changesmade = 0;
//    char key;
//    char* page;
//    unsigned char pagenr;
//    BYTE yesno;
//    BYTE selected = 0;
//
//    clrscr();
//    headertext("Delete menu slots");
//
//    presentmenuslots();
//
//    gotoxy(0,23);
//    cputs("Choose menu slot to be deleted. ");
//
//    do
//    {
//        key = cgetc();
//        if ((key>47 && key<58) || (key>64 && key<91))    // If keys 0 - 9 or a - z
//        {
//            menuslot = keytomenuslot(key);
//            selected = 1;
//        }
//    } while (selected == 0);
//
//    getslotfromem(menuslot);
//
//    cprintf("%c\n\r", key);
//    if ( strlen(Slot.menu) != 0 )
//    {
//        cprintf("Are you sure? Y/N ");
//        yesno = getkey(128);
//        cprintf("%c", yesno);
//        if ( yesno == 78 )
//        {
//            selected = 0;
//        }
//    }
//    else
//    {
//        cprintf("Slot is already empty. Press key.");
//        getkey(2);
//        selected = 0;
//    }
//    if (selected == 1)
//    {
//        pagenr = menuslot * 2;
//        page = em_use(pagenr);
//        memset(page,0,256);
//        em_commit();
//        pagenr++;
//        page = em_use(pagenr);
//        memset(page,0,256);
//        em_commit();
//        changesmade = 1;
//    }
//    
//    return changesmade;
//}
//
//int renamemenuslot()
//{
//    // Routine to rename a chosen menu slot
//    // Returns 1 if something has been renamed, else 0
//
//    int menuslot;
//    int changesmade = 0;
//    char key;
//    BYTE yesno;
//    BYTE selected = 0;
//
//    clrscr();
//    headertext("Rename menu slots");
//
//    presentmenuslots();
//
//    gotoxy(0,21);
//    cputs("Choose menu slot to be renamed. ");
//
//    do
//    {
//        key = cgetc();
//        if ((key>47 && key<58) || (key>64 && key<91))    // If keys 0 - 9 or a - z
//        {
//            menuslot = keytomenuslot(key);
//            selected = 1;
//        }
//    } while (selected == 0);
//
//    getslotfromem(menuslot);
//    
//    cprintf("%c\n\r", key);
//    if ( strlen(Slot.menu) != 0 )
//    {
//        cprintf("Are you sure? Y/N ");
//        yesno = getkey(128);
//        cprintf("%c\n\r", yesno);
//        if ( yesno == 78 )
//        {
//            selected = 0;
//        }
//    }
//    else
//    {
//        cprintf("Slot is empty. Press key.");
//        getkey(2);
//        selected = 0;
//    }
//    if (selected == 1)
//    {
//        gotoxy(0,23);
//        cputs("Choose name for slot:");
//        textInput(0,24,Slot.menu,20);
//        putslottoem(menuslot);
//        changesmade = 1;
//    }
//    
//    return changesmade;
//}
//
//int reordermenuslot()
//{
//    // Routine to reorder a chosen menu slot
//    // Returns 1 if something has been renamed, else 0
//
//    int menuslot;
//    int newslot;
//    int changesmade = 0;
//    char key;
//    BYTE select = 0;
//    char menubuffer[21];
//    int oldslotbuffer;
//    int x;
//    int xpos;
//    int ypos;
//    int maxpos;
//
//    do
//    {
//        clrscr();
//        headertext("Re-order menu slots");
//
//        presentmenuslots();
//
//        cputsxy(0,22,"Choose menu slot to be re-ordered.");
//        cputsxy(0,23,"Or choose ");
//        revers(1);
//        textcolor(DMB_COLOR_SELECT);
//        cputs(" F7 ");
//        revers(0);
//        textcolor(DC_COLOR_TEXT);
//        cputs(" to return to main menu.");
//
//        do
//        {
//            key = cgetc();    // obtain alphanumeric key
//            if (key == CH_F7)
//            {
//                select = 1;
//            }
//            else
//            {
//                if ((key>47 && key<58) || (key>64 && key<91))   // If keys 0-9,a-z
//                {
//                    getslotfromem(keytomenuslot(key));
//                    if(strlen(Slot.menu) != 0)   // Check if menslot is empty
//                    {
//                        select = 1;
//                    }
//                }
//            }
//        } while (select == 0);
//
//        if (key != CH_F7)
//        {
//            clearArea(0,22,40,2);
//            menuslot = keytomenuslot(key);
//            getslotfromem(menuslot);
//            if (menuslot>17)
//            {
//                xpos = 40;
//                ypos = menuslot-15;
//            }
//            else
//            {
//                xpos = 0;
//                ypos = menuslot+3;
//            }            
//            gotoxy(xpos,ypos);
//            revers(1);
//            textcolor(DC_COLOR_HIGHLIGHT);
//            cprintf(" %2c ",menuslotkey(menuslot));
//            revers(0);
//            cprintf(" %s",Slot.menu);
//            gotoxy(0,22);
//            textcolor(DC_COLOR_TEXT);
//            cputs("Move slot up or down by ");
//            textcolor(DMB_COLOR_SELECT);
//            cputs("cursor keys.\n\r");
//            cputs("ENTER");
//            textcolor(DC_COLOR_TEXT);
//            cputs(" to confirm position, ");
//            textcolor(DMB_COLOR_SELECT);
//            cputs("F7");
//            textcolor(DC_COLOR_TEXT);
//            cputs(" to cancel.");
//
//            for (x=0;x<36;x++)
//            {
//                getslotfromem(x);
//                putslottoem(x+40);
//                strcpy(newmenuname[x],Slot.menu);
//                newmenuoldslot[x] = x;
//            }
//
//            newslot = menuslot;
//
//            if (SCREENW==40)
//            {
//                maxpos = 14;
//            }
//            else
//            {
//                maxpos = 35;
//            }
//            
//            do
//            {
//
//                do
//                {
//                    key = cgetc();
//                } while (key != CH_F7 && key !=13 && key !=17 && key !=145);
//
//                switch (key)
//                {
//                case 17:
//                    if (newslot == maxpos)
//                    {
//                        strcpy(menubuffer, newmenuname[0]);
//                        strcpy(newmenuname[0], newmenuname[maxpos]);
//                        strcpy(newmenuname[maxpos], menubuffer);
//                        oldslotbuffer = newmenuoldslot[0];
//                        newmenuoldslot[0] = newmenuoldslot[maxpos];
//                        newmenuoldslot[maxpos] = oldslotbuffer;
//                        printnewmenuslot(maxpos,0,newmenuname[maxpos]);
//                        printnewmenuslot(0,1,newmenuname[0]);
//                        newslot = 0;
//                    }
//                    else
//                    {
//                        strcpy(menubuffer, newmenuname[newslot+1]);
//                        strcpy(newmenuname[newslot+1], newmenuname[newslot]);
//                        strcpy(newmenuname[newslot], menubuffer);
//                        oldslotbuffer = newmenuoldslot[newslot+1];
//                        newmenuoldslot[newslot+1] = newmenuoldslot[newslot];
//                        newmenuoldslot[newslot] = oldslotbuffer;
//                        printnewmenuslot(newslot,0,newmenuname[newslot++]);
//                        printnewmenuslot(newslot,1,newmenuname[newslot]);
//                    }                 
//                    break;
//
//                case 145:
//                    if (newslot == 0)
//                    {
//                        strcpy(menubuffer, newmenuname[maxpos]);
//                        strcpy(newmenuname[maxpos], newmenuname[0]);
//                        strcpy(newmenuname[0], menubuffer);
//                        oldslotbuffer = newmenuoldslot[maxpos];
//                        newmenuoldslot[maxpos] = newmenuoldslot[0];
//                        newmenuoldslot[0] = oldslotbuffer;
//                        printnewmenuslot(0,0,newmenuname[0]);
//                        printnewmenuslot(maxpos,1,newmenuname[maxpos]);
//                        newslot = maxpos;
//                    }
//                    else
//                    {
//                        strcpy(menubuffer, newmenuname[newslot-1]);
//                        strcpy(newmenuname[newslot-1], newmenuname[newslot]);
//                        strcpy(newmenuname[newslot], menubuffer);
//                        oldslotbuffer = newmenuoldslot[newslot-1];
//                        newmenuoldslot[newslot-1] = newmenuoldslot[newslot];
//                        newmenuoldslot[newslot] = oldslotbuffer;
//                        printnewmenuslot(newslot,0,newmenuname[newslot--]);
//                        printnewmenuslot(newslot,1,newmenuname[newslot]);
//                    }
//                    break;
//                
//                default:
//                    break;
//                }                
//            } while (key != CH_F7 && key != 13);
//
//            if (key == 13)
//            {
//                changesmade = 1;
//                for (x=0;x<36;x++)
//                {
//                    getslotfromem(newmenuoldslot[x]);
//                    strcpy(Slot.menu,newmenuname[x]);
//                    putslottoem(x+40);
//                }
//                for (x=0;x<36;x++)
//                {
//                    getslotfromem(x+40);
//                    putslottoem(x);
//                }
//            }
//        }
//    } while (key != CH_F7);
//
//    return changesmade;
//}
//
//void printnewmenuslot(int pos, int color, char* name)
//{
//    // Routine to print menuslot item
//    // Input: color for slotnumber
//    // 0 Selectable text color
//    // 1 Selected text color
//
//    int xpos;
//    int ypos;
//
//    if (pos>17)
//    {
//        xpos = 40;
//        ypos = pos-15;
//    }
//    else
//    {
//        xpos = 0;
//        ypos = pos+3;
//    }
//
//    clearArea(xpos,ypos,40,1);
//    gotoxy(xpos,ypos);
//    revers(1);
//    if (color == 0)
//    {
//        textcolor(DMB_COLOR_SELECT);
//    }
//    else
//    {
//        textcolor(DC_COLOR_HIGHLIGHT);
//    }
//    cprintf(" %2c ",menuslotkey(pos));
//    revers(0);
//    if (color == 0)
//    {
//        textcolor(DC_COLOR_TEXT);
//    }
//    else
//    {
//        textcolor(DC_COLOR_HIGHLIGHT);
//    }
//    if ( strlen(name) == 0 )
//    {
//        cputs(" <EMPTY>");
//    }
//    else
//    {
//        cprintf(" %s",name);
//    }
//}
//
//void getslotfromem(int slotnumber)
//{
//    // Routine to read a menu option from extended memory page
//    // Input: Slotnumber 0-36 (or 40-76 for backup)
//
//    char* page;
//    unsigned char pagenr = slotnumber * 2;
//
//    // Calculate page address from slotnumber 
//    page = em_map(pagenr);
//
//    // Copy data from first page
//    strcpy(Slot.path, page);
//    page += 100;
//    strcpy(Slot.menu, page);
//    page += 21;
//    strcpy(Slot.file, page);
//    page += 20;
//    strcpy(Slot.cmd, page);
//    page += 80;
//    strcpy(Slot.reu_image, page);
//    page += 20;
//    Slot.reusize = *page;
//    page++;
//    Slot.runboot = *page;
//    page++;
//    Slot.device = *page;
//    page++;
//    Slot.command  = *page;
//    page++;
//    Slot.cfgvs  = *page;
//
//    // Copy data from second page
//    pagenr++;
//    page = em_map(pagenr);
//    strcpy(Slot.image_a_path, page);
//    page += 100;
//    strcpy(Slot.image_a_file, page);
//    page += 20;
//    Slot.image_a_id = *page;
//    page++;
//    strcpy(Slot.image_b_path, page);
//    page += 100;
//    strcpy(Slot.image_b_file, page);
//    page += 20;
//    Slot.image_b_id = *page;
//}
//
//void putslottoem(int slotnumber)
//{
//    // Routine to write a menu option to extended memory page
//    // Input: Slotnumber 0-36 (or 40-76 for backup)
//    char* page;
//    unsigned char pagenr = slotnumber * 2;
//
//    // Point at first page and erase page
//    page = em_use(pagenr);
//    memset(page,0,256);
//
//    // Store data in first page
//    strcpy(page, Slot.path);
//    page += 100;
//    strcpy(page, Slot.menu);
//    page += 21;
//    strcpy(page, Slot.file);
//    page += 20;
//    strcpy(page, Slot.cmd);
//    page += 80;
//    strcpy(page, Slot.reu_image);
//    page += 20;
//    *page = Slot.reusize;
//    page++;
//    *page = Slot.runboot;
//    page++;
//    *page = Slot.device;
//    page++;
//    *page = Slot.command;
//    page++;
//    *page = Slot.cfgvs;  
//    em_commit();
//
//    // Point at first page and erase page
//    pagenr++;
//    page = em_use(pagenr);
//    memset(page,0,256);
//
//    // Store data in second page
//    page = em_use(pagenr);
//    strcpy(page, Slot.image_a_path);
//    page += 100;
//    strcpy(page, Slot.image_a_file);
//    page += 20;
//    *page = Slot.image_a_id;
//    page++;
//    strcpy(page, Slot.image_b_path);
//    page += 100;
//    strcpy(page, Slot.image_b_file);
//    page += 20;
//    *page = Slot.image_b_id;
//    em_commit();
//}
//
//char menuslotkey(int slotnumber)
//{
//    // Routine to convert numerical slotnumber to key in menu
//    // Input: Slotnumber = menu slot number
//    // Output: Corresponding 0-9, a-z key
//
//    if(slotnumber<10)
//    {
//        return slotnumber+48; // Numbers 0-9
//    }
//    else
//    {
//        return slotnumber+87; // Letters a-z
//    }
//}
//
//int keytomenuslot(char keypress)
//{
//    // Routine to convert keypress to numerical slotnumber
//    // Input: keypress = ASCII value of key pressed 0-9 or a-z
//    // Output: Corresponding menuslotnumber
//
//    if(keypress>64)
//    {
//        return keypress - 55;
//    }
//    else
//    {
//        return keypress - 48;
//    }
//}
//
//int edituserdefinedcommand()
//{
//    // Routine to edit user defined command in menuslot
//    // Returns 1 if something has been renamed, else 0
//
//    int menuslot;
//    int changesmade = 0;
//    unsigned char key;
//    BYTE selected = 0;
//
//    clrscr();
//    headertext("Edit user defined command");
//
//    presentmenuslots();
//
//    gotoxy(0,21);
//    cputs("Choose menu slot to edit. ");
//
//    do
//    {
//        key = cgetc();
//        if ((key>47 && key<58) || (key>64 && key<91))    // If keys 0 - 9 or a - z
//        {
//            menuslot = keytomenuslot(key);
//            selected = 1;
//        }
//    } while (selected == 0);
//
//    getslotfromem(menuslot);
//    
//    cprintf("%c\n\r", key);
//    if ( strlen(Slot.menu) == 0 )
//    {
//        cprintf("Slot is empty. Press key.");
//        getkey(2);
//        selected = 0;
//    }
//    if (selected == 1)
//    {
//        clrscr();
//        headertext("Edit user defined command");
//
//        gotoxy(0,3);
//        cputs("Chosen slot:\n\r");
//        revers(1);
//        textcolor(DMB_COLOR_SELECT);
//        cprintf(" %c ",menuslotkey(menuslot));
//        revers(0);
//        textcolor(DC_COLOR_TEXT);
//        cprintf(" %s",Slot.menu);
//
//        cputsxy(0,6,"Enter command (empty=none):");
//        textInput(0,7,Slot.cmd,79);
//        if( strlen(Slot.cmd) == 0)
//        {
//            Slot.command = 0;
//        }
//        else
//        {
//            Slot.command = 1;
//        }
//        
//        putslottoem(menuslot);
//        changesmade = 1;
//    }
//    
//    return changesmade;
//}

void information()
{
    // Routine for version information and credits

    clrscr();
    headertext("Information and credits");

    cputs("\n\rUBoot64: Boot menu for Ultimate devices\n\n\r");
    cprintf("Version: v%i%i-", VERSION_MAJOR, VERSION_MINOR);
    cprintf("%c%c%c%c", BUILD_YEAR_CH0, BUILD_YEAR_CH1, BUILD_YEAR_CH2, BUILD_YEAR_CH3);
    cprintf("%c%c%c%c-", BUILD_MONTH_CH0, BUILD_MONTH_CH1, BUILD_DAY_CH0, BUILD_DAY_CH1);
    cprintf("%c%c%c%c\n\r", BUILD_HOUR_CH0, BUILD_HOUR_CH1, BUILD_MIN_CH0, BUILD_MIN_CH1);
    cputs("Written 2023 by Xander Mol.\n\n\r");
    cputs("Inspired by/code used of DraBrowse:\n\r");
    cputs("DraBrowse is a simple file browser.\n\r");
    cputs("Original 2009 by Sascha Bader.\n\r");
    cputs("Used version adapted by Dirk Jagdmann.\n\n\r");
    cputs("Requires and made possible by:\n\n\r");
    cputs("The Ultimate II+ cartridge,\n\r");
    cputs("Created by Gideon Zweijtzer.\n\n\r");

    cputs("Press a key to continue.");

    getkey(2);    
}

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

        uii_change_dir("/usb*/");
	    printf("\nDir changed\nStatus: %s", uii_status);

	    readconfigfile(configfilename);

        // Load slot config
        std_read("dmbslt.cfg",1); // Read config file

        // Set time from NTP server
        time_main();
    } else {
        // Restore slots in memory returning from filebrowser
        std_read("dmbslt.cfg",0);
        //pickmenuslot();
    }

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
    //    case CH_F1:
    //        // Filebrowser
    //        bankrun(1);  // Jump to bank of filebrowser and start entry point
    //        break;
    //    
    //    case CH_F4:
    //        config_main();
    //        break;
//
        case CH_F2:
            // Information and credits
            information();
            break;
        
    //    case CH_F7:
    //        // Edit / re-order and delete menuslots
    //        editmenuoptions();
    //        break;
    //    
        default:
            break;
        }
    } while (menuselect != CH_F3);

    exitScreen();
    bankout();
}

#pragma rodata-name	("RODATA");

const struct codetable codetable [] =
{
	{&main,		1},
	{&mainmenu,		1}
};

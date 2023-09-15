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
#include "u-time.h"
#include "ultimate_common_lib.h"
#include "ultimate_dos_lib.h"
#include "ultimate_time_lib.h"
#include "ultimate_network_lib.h"
#include "fc3.h"

#pragma code-name	("CODE2");
#pragma rodata-name	("RODATA2");

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

char* UNIX_epoch_to_UII_time(time_t epoch) {
// Convert UNIX time epoch to UII readable time format

    static unsigned char month_days[12]={31,28,31,30,31,30,31,31,30,31,30,31};

    unsigned char ntp_hour, ntp_minute, ntp_second, ntp_day, ntp_month;
    unsigned char leap_days=0;
    unsigned char leap_year_ind=0;
    unsigned int temp_days, i;
    unsigned int ntp_year, days_since_epoch, day_of_year;
    char settime[6];

    // Adjust for timezone
    epoch+=secondsfromutc;

    // Calculate time
    ntp_second = epoch%60;
    epoch /= 60;
    ntp_minute = epoch%60;
    epoch /= 60;
    ntp_hour  = epoch%24;
    epoch /= 24;

    // Calculate date

    // Number of days since epoch
    days_since_epoch = epoch;
    // ball parking year, may not be accurate!
    ntp_year = 1970+(days_since_epoch/365);
    // Calculating number of leap days since epoch/1970
    for (i=1972; i<ntp_year; i+=4) {                                
        if(((i%4==0) && (i%100!=0)) || (i%400==0)) leap_days++;
    }
    // Calculating accurate current year by (days_since_epoch - extra leap days)
    ntp_year = 1970+((days_since_epoch - leap_days)/365);
    day_of_year = ((days_since_epoch - leap_days)%365)+1;

    if(((ntp_year%4==0) && (ntp_year%100!=0)) || (ntp_year%400==0)) {
        month_days[1]=29;     //February = 29 days for leap years
        leap_year_ind = 1;    //if current year is leap, set indicator to 1 
    }
    else { month_days[1]=28; } //February = 28 days for non-leap years 

    // Calculating current Month
    temp_days=0;
    for (ntp_month=0 ; ntp_month <= 11 ; ntp_month++) {
        if (day_of_year <= temp_days) break; 
        temp_days = temp_days + month_days[ntp_month];
    }

    // Calculating current Date
    temp_days = temp_days - month_days[ntp_month-1];
    ntp_day = day_of_year - temp_days;

    // Build UII time
    settime[0]=ntp_year-1900;
    settime[1]=ntp_month;
    settime[2]=ntp_day;
    settime[3]=ntp_hour;
    settime[4]=ntp_minute;
    settime[5]=ntp_second;

    return settime;
}

void get_ntp_time()
{
    // Function to get time from NTP server and set UII+ time with this

    unsigned char attempt = 1;
    unsigned char clock;
    unsigned char fullcmd[] = { 0x00, NET_CMD_SOCKET_WRITE, 0x00, \
                               0x1b, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, \
                               0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, \
                               0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    unsigned char socket = 0;
    time_t t;

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
        cia_seconds = 0;
        cia_tensofsec = 0;
        clock = cia_seconds;
        while (cia_seconds == clock) { ; }
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
    
    // Set UII+ RTC clock
    uii_set_time(UNIX_epoch_to_UII_time(t));    
    printf("\nStatus: %s", uii_status);
    uii_get_time();
    printf("\nRTC clock set to %s",uii_data);
}

void time_main()
{
    if(timeonflag == 1)
    {
        get_ntp_time();
    }
}

// Config functions
void edittimeconfig()
{
    unsigned char changesmade = 0;
    unsigned char key;
    char offsetinput[10] = "";
    char buffer2[80] = "";
    char* ptrend;

    clrscr();
    headertext("Configuration tool.");
    gotoxy(0,3);
    cputs("Present configuration settings:\n\n\r");
    cputs("NTP time update settings:\n\r");
    cprintf("- Update on boot toggle: %s\n\r",(timeonflag==0)?"Off":"On");
    cprintf("- Offset to UTC in seconds: %ld\n\r",secondsfromutc);
    mid(host,0,SCREENW,buffer2,SCREENW);
    cprintf("- NTP server hostname:\n\r%s\n\r",buffer2);

    gotoxy(0,18);
    cputs("Make your choice:\n\r");

    revers(1);
    textcolor(DMB_COLOR_SELECT);
    cputs(" F1 ");
    revers(0);
    textcolor(DC_COLOR_TEXT);
    cputs(" Toggle update on boot on/off\n\r");

    revers(1);
    textcolor(DMB_COLOR_SELECT);
    cputs(" F3 ");
    revers(0);
    textcolor(DC_COLOR_TEXT);
    cputs(" Edit time offset to UTC\n\r");

    revers(1);
    textcolor(DMB_COLOR_SELECT);
    cputs(" F5 ");
    revers(0);
    textcolor(DC_COLOR_TEXT);
    cputs(" Edit NTP server host\n\r");

    revers(1);
    textcolor(DMB_COLOR_SELECT);
    cputs(" F7 ");
    revers(0);
    textcolor(DC_COLOR_TEXT);
    cputs(" Back to main menu\n\r");

    do
    {
        do
        {
            key = cgetc();
        } while (key != CH_F1 && key != CH_F3 && key != CH_F5 && key != CH_F7);

        switch (key)
        {
        case CH_F1:
            timeonflag = (timeonflag==0)? 1:0;
            gotoxy(0,6);
            cprintf("- Update on boot toggle: %s\n\r",(timeonflag==0)?"Off":"On ");
            changesmade = 1;
            break;

        case CH_F3:
            cputsxy(0,23,"Input time offset to UTC:");
            textInput(0,24,offsetinput,10);
            secondsfromutc = strtol(offsetinput,&ptrend,10);
            clearArea(0,7,40,1);
            clearArea(0,23,40,2);
            gotoxy(0,7);
            cprintf("- Offset to UTC in seconds: %ld\n\n\r",secondsfromutc);
            changesmade = 1;
            break;

        case CH_F5:
            cputsxy(0,23,"Input NTP server hostname:");
            textInput(0,24,host,79);
            clearArea(0,9,40,1);
            clearArea(0,23,40,2);
            gotoxy(0,9);
            mid(host,0,SCREENW,buffer2,SCREENW);
            cprintf("%s",buffer2);
            changesmade = 1;
            break;

        default:
	    	break;
        }
    } while (key != CH_F7);

    if (changesmade == 1)
    {
        writeconfigfile(configfilename);
    }
}
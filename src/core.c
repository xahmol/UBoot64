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
#include "defines.h"
#include "core.h"
#include "ultimate_common_lib.h"
#include "ultimate_dos_lib.h"
#include "ultimate_time_lib.h"
#include "fc3.h"

const char *value2hex = "0123456789abcdef";
const char *reg_types[] = { "SEQ","PRG","URS","REL","VRP" };
const char *oth_types[] = { "DEL","CBM","DIR","LNK","OTH","HDR"};
char bad_type[4];

BYTE device = 8;
char linebuffer[100];
char linebuffer2[100];
char DOSstatus[40];

/// string descriptions of enum drive_e
const char* drivetype[LAST_DRIVE_E] = {"", "Pi1541", "1540", "1541", "1551", "1570", "1571", "1581", "1001", "2031", "8040", "sd2iec", "cmd", "vice", "u64"};/// enum drive_e value for each device 0-19.
BYTE devicetype[MAXDEVID+1];

// Generic functions
void errorexit()
{
  // Bank out on error after keypress
  printf("\n\rPress key to exit to BASIC.");
  cgetc();
  bankout();
}

void StringSafeCopy(char* dest, char* src, unsigned char maxlen) {
    // Function to safely copy a string up to a maximum length to destination string
    // Ensure it is always zero terminated

    strncpy(dest,src,maxlen);
    if(strlen(src)>maxlen) { dest[maxlen]=0; }
}

void mid(const char *src, size_t start, size_t length, char *dst, size_t dstlen)
{       
    // Function to provide MID$ equivalent

    size_t len = min( dstlen - 1, length);
 
    strncpy(dst, src + start, len);
    // zero terminate because strncpy() didn't ? 
    if(len < length)
        dst[dstlen-1] = 0;
}

void cspaces(unsigned char number)
{
    /* Function to print specified number of spaces, cursor set by conio.h functions */

    unsigned char x;

    for(x=0;x<number;x++) { cputc(32); }
}

char* pathconcat()
{
    // Function to concatinate the path strings

    char concat[100] ="";
    int x;

    if(fb_uci_mode) {
        uii_get_path();
        StringSafeCopy(concat,uii_data,99);
    } else {
        if ( devicetype[pathdevice] == VICE || devicetype[pathdevice] == U64)
        {
            strcat( concat, "cd:/");
        }
        else
        {
            strcat( concat, "cd//");
        }
        for (x=0 ; x < depth ; ++x)
        {
            strcat( concat, path[x] );
            strcat( concat, "/");
        }
    }

    return concat;
}

char getkey(BYTE mask)
{
    // Function to wait for key within input validation mask
    // Mask values for input validation (adds up for combinations):
    // 00000001 =   1 = Numeric
    // 00000010 =   2 = Alpha lowercase
    // 00000100 =   4 = Alpha uppercase
    // 00001000 =   8 = Up and down
    // 00010000 =  16 = Left and right
    // 00100000 =  32 = Delete and insert
    // 01000000 =  64 = Return
    // 10000000 = 128 = Y and N

    BYTE keychar;

    do
    {
        keychar = cgetc();
    } while ( !(mask&1 && keychar > 47 && keychar < 58) && !(mask&2 && keychar > 31 && keychar < 96) && !(mask&4 && keychar > 95 && keychar < 128) && !(mask&16 && (keychar == 29 || keychar == 157)) && !(mask&8 && (keychar == 17 || keychar == 145)) && !(mask&32 && (keychar == 20 || keychar == 148)) && !(mask&64 && keychar == 13) && !(mask&128 && (keychar == 78 || keychar == 89)) );
    return keychar;    
}

void clearArea(const BYTE xpos, const BYTE ypos, const BYTE xsize, const BYTE ysize)
{
  BYTE y = ypos;
  for (; y < (ypos+ysize); ++y)
    {
      cclearxy(xpos,y,xsize);
    }
}

/* initialize screen mode */
void initScreen(const BYTE border, const BYTE bg, const BYTE text)
{
  bordercolor(border);
  bgcolor(bg);
  textcolor(text);
  clrscr();
}

void headertext(char* subtitle)
{
    // Draw header text
    // Input: subtitle is text to draw on second line

    revers(1);
    textcolor(DMB_COLOR_HEADER1);
    gotoxy(0,0);
    cspaces(SCREENW);
    gotoxy(0,0);  
    cprintf("UBoot64:  Boot Menu for Ultimate devices");
    textcolor(DMB_COLOR_HEADER2);
    gotoxy(0,1);
    cspaces(SCREENW);
    gotoxy(0,1);
    cprintf("%s\n\n\r", subtitle);
    uii_get_time();
    cputsxy(SCREENW-strlen((const char*)uii_data),1,(const char*)uii_data);
    revers(0);
    textcolor(DC_COLOR_TEXT);
}

// IO routines
unsigned char CheckIfUltimateOnID(unsigned char id) {
// Check if an ultimate device exists for which powertoggle by UCI is not possible
// Bit 0: Exists yes or no
// Bit 1: Powered on yes or no
// Bit 2: UCI controllabke yes or no

  unsigned char x;
  unsigned char checkvalue;

  for(x=0;x<4;x++) {
    if(uii_devinfo[x].id == id) {
      checkvalue = 1;                               // Set bit 0 if exists
      checkvalue += (uii_devinfo[x].power)?2:0;     // Set bit 1 if powered
      checkvalue += (uii_devinfo[x].type<0x0f)?4:0; // Set bit 2 if UCI controllable
      return checkvalue;
    }
  }
  return 0;                                         // No Ultimate device on ID
}

unsigned char CheckActiveIECdevices() {
// Check all non drivve A and B IEC devices if they are active or not
// Fill iec_devices array and return 1 if any is active, 0 for none

  unsigned char anyactive = 0;
  unsigned char x,check;

  // Wipe array
  memset(iec_devices,0,sizeof(iec_devices));

  // First get Ultimate devices
  if(!uii_parse_deviceinfo()) { errorexit(); }

  // Check IDs 8 to 30
  for(x=0;x<23;x++) {
    iec_device = (x==22)?4:x+8;
    check = (CheckIfUltimateOnID(iec_device));
    //printf("C%d ",check);
    if(check) {
      if(check > 1) {
        iec_devices[x] = 1;
        anyactive = (check==3)?1:0; // Set anyactive if not UCI controllable and powered
      }
    } else {
      iec_present();
      //printf("I%d ",iec_device);
      if(iec_device) {
        iec_devices[x] = 1;
        anyactive = 1;
      }
    }
    //printf("%2d:%d ",iec_device,iec_devices[x]);
  }

  return anyactive;
}

BYTE dosCommand(const BYTE lfn, const BYTE drive, const BYTE sec_addr, const char *cmd)
{
  int res;
  if (cbm_open(lfn, drive, sec_addr, cmd) != 0)
    {
      cbm_close(lfn);
      return _oserror;
    }

  if (lfn != 15)
    {
      if (cbm_open(15, drive, 15, "") != 0)
        {
          cbm_close(lfn);
          return _oserror;
        }
    }

  DOSstatus[0] = 0;
  res = cbm_read(15, DOSstatus, sizeof(DOSstatus));

  if(lfn != 15)
    {
      cbm_close(15);
    }
  cbm_close(lfn);

  if (res < 1)
    {
      return _oserror;
    }

  return (DOSstatus[0] - 48) * 10 + DOSstatus[1] - 48;
}

int cmd(const BYTE device, const char *cmd)
{
  return dosCommand(15, device, 15, cmd);
}

const char* getDeviceType(const BYTE device) {
  BYTE idx;

  if (device > sizeof(devicetype))
    {
      return "!d";
    }
  idx = cmd(device, "ui");
  if (idx != 73)
    {
      linebuffer2[0] = 'Q';
      linebuffer2[1] = value2hex[idx >> 4];
      linebuffer2[2] = value2hex[idx & 15];
      linebuffer2[3] = 0;
      return linebuffer2;
    }
  for(idx = 1; idx < LAST_DRIVE_E; ++idx)
    {
      if(strstr(DOSstatus, drivetype[idx]))
        {
          devicetype[device] = idx;
          return drivetype[idx];
        }
    }
  return "!n";
}

void execute(char * prg, BYTE device, BYTE boot, char * command)
{
  // Routine to execute or boot chosen file or dir
  // Input:
  // prg:     Filename
  // device:  device number
  // boot:    Execute flag
  //          bit 0: Mount flag
  //          bit 1: Load with ,1
  // command: User defined command to be executed before execution.
  //          Empty is no command.

  unsigned int pos=2;
  unsigned char numberenter = 2;
  unsigned char x;
  
  // First output two enters
  execute_commands[0] = 0x0d;
  execute_commands[1] = 0x0d;

  // Output user defined command if needed
  if (strlen(command) != 0)
  {
    StringSafeCopy(execute_commands+pos, command, 199-pos);
    pos = strlen(command);
    execute_commands[pos++] = 0x0d;
    numberenter++;
  }

  // Output load and run commands
  if(boot&EXEC_COMMA1) {
    // Load with ,1
    sprintf(execute_commands+pos,"load\"%s\",%i,1%c%c%c%c%crun%c%c", prg, device,0x0d,0x0d,0x0d,0x0d,0x0d,0x0d,0);
  } else {
    // Load without ,1
    sprintf(execute_commands+pos,"load\"%s\",%i%c%c%c%c%crun%c%c", prg, device,0x0d,0x0d,0x0d,0x0d,0x0d,0x0d,0);
  }

  // put CRs in keyboard buffer staging area
  for(x=0;x<numberenter;x++)
  {
    execute_keys[x] = 0x0d;
  }
  execute_keys[x]=0;
    
  // exit DraCopy, which will execute the BASIC LOAD above
  bankout();
}

/**
 * input/modify a string.
 * based on version 1.0e, then modified.
 * @param[in] xpos screen x where input starts.
 * @param[in] ypos screen y where input starts.
 * @param[in,out] str string that is edited, it can have content and must have at least @p size + 1 bytes. Maximum size if 255 bytes.
 * @param[in] size maximum length of @p str in bytes.
 * @return -1 if input was aborted.
 * @return >= 0 length of edited string @p str.
 */
int textInput(const BYTE xpos, const BYTE ypos, char *str, const BYTE size)
{
  register BYTE idx = strlen(str);
  register BYTE c;

  cursor(1);
  cputsxy(xpos, ypos, str);

  while(1)
    {
      c = cgetc();
      switch (c)
        {
      case CH_LARROW:
      case CH_ESC:
        cursor(0);
        return -1;

      case CH_ENTER:
        idx = strlen(str);
        str[idx] = 0;
        cursor(0);
        return idx;

      case CH_DEL:
        if (idx)
          {
            --idx;
            cputcxy(xpos + idx, ypos, ' ');
            for(c = idx; 1; ++c)
              {
                const BYTE b = str[c+1];
                str[c] = b;
                cputcxy(xpos + c, ypos, b ? b : ' ');
                if (b == 0)
                  break;
              }
            gotoxy(xpos + idx, ypos);
          }
        break;

        case CH_INS:
          c = strlen(str);
          if (c < size &&
              c > 0 &&
              idx < c)
            {
              ++c;
              while(c >= idx)
                {
                  str[c+1] = str[c];
                  if (c == 0)
                    break;
                  --c;
                }
              str[idx] = ' ';
              cputsxy(xpos, ypos, str);
              gotoxy(xpos + idx, ypos);
            }
          break;

      case CH_CURS_LEFT:
        if (idx)
          {
            --idx;
            gotoxy(xpos + idx, ypos);
          }
        break;

      case CH_CURS_RIGHT:
        if (idx < strlen(str) &&
            idx < size)
          {
            ++idx;
            gotoxy(xpos + idx, ypos);
          }
        break;

      default:
        if (isprint(c) &&
            idx < size)
          {
            const BYTE flag = (str[idx] == 0);
            str[idx] = c;
            cputc(c);
            ++idx;
            if (flag)
              str[idx+1] = 0;
            break;
          }
        break;
      }
    }
  return 0;
}

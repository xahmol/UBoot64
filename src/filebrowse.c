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
#include "petscii_ascii.h"
#include "ultimate_common_lib.h"
#include "ultimate_dos_lib.h"
#include "fc3.h"

#pragma code-name	("CODE3");
#pragma rodata-name	("RODATA3");

#define CBM_T_FREE 100

#define DISK_ID_LEN 5
#define disk_id_buf linebuffer2

static const char progressBar[4] = { 0xA5, 0xA1, 0xA7, ' ' };
static const char progressRev[4] = { 0,    0,    1,    1 };

struct DirElement {
  struct cbm_dirent dirent;
  struct DirElement * next;
  struct DirElement * prev;
  unsigned char flags;
};
struct DirElement direlement_size;
struct DirElement *previous;
struct DirElement *current;
struct DirElement *next;

struct Directory {
  /// 16 characters name
  /// 1 comma
  /// 5 characters ID
  /// NUL
  char name[16+1+5+1];
  struct DirElement * firstelement;
  struct DirElement * selected;
  struct DirElement * firstprinted;
  /// current cursor position
  unsigned int pos;
  /// number of free blocks
  unsigned int free;
};
struct Directory cwd;

static BYTE sorted = 0;

void freeDir() {
// Free memory of directory structure
    current = cwd.firstelement;

    while (current)
    {
      next = current->next;
      free(current);
      current = next;
    }
}

unsigned char myCbmReadDir(const BYTE device, struct cbm_dirent* l_dirent) {
/**
 * @param l_dirent pointer to cbm_dirent object, must be memset to 0.
 * @return 0 upon success, @p l_dirent was set.
 * @return >0 upon error.
 */

  BYTE b, len;
  BYTE i = 0;

  // check that device is ready
  if (cbm_k_chkin (device) != 0)
    {
      cbm_k_clrch();
      return 1;
    }
  if (cbm_k_readst() != 0)
    {
      return 7;
    }

  // skip next basic line: 0x01, 0x01
  cbm_k_basin();
  cbm_k_basin();

  // read file size
  l_dirent->size = cbm_k_basin();
  l_dirent->size |= (cbm_k_basin()) << 8;

  // read line into linebuffer
  memset(linebuffer, 0, sizeof(linebuffer));
  //cclearxy(0,BOTTOM,SCREENW);//debug
  while(1)
    {
      // read byte
      b = cbm_k_basin();
      // EOL?
      if (b == 0)
        {
          break;
        }
      // append to linebuffer
      if (i < sizeof(linebuffer))
        {
          linebuffer[i++] = b;
          //cputcxy(i,BOTTOM,b);//debug
        }
      // return if reading had error
      if (cbm_k_readst() != 0)
        {
          cbm_k_clrch();
          return 2;
        }
    }
  cbm_k_clrch();
  //cputcxy(i,BOTTOM,'?');//debug

  // handle "B" BLOCKS FREE
  if (linebuffer[0] == 'b')
    {
      l_dirent->type = CBM_T_FREE;
      return 0;
    }

  // check that we have a minimum amount of characters to work with
  if (i < 5)
    {
      return 3;
    }

  // strip whitespace from right part of line
  for(len = i; len > 0; --len)
    {
      b = linebuffer[len];
      if (b == 0 ||
          b == ' ' ||
          b == 0xA0)
        {
          linebuffer[len] = 0;
          continue;
        }
      ++len;
      break;
    }

  //cputcxy(len,BOTTOM,'!');//debug
  //cgetc();//debug

  // parse file name

  // skip until first "
  for(i = 0; i < sizeof(linebuffer) && linebuffer[i] != '"'; ++i)
    {
      // do nothing
    }

  // copy filename, until " or max size
  b = 0;
  for(++i; i < sizeof(linebuffer) && linebuffer[i] != '"' && b < 16; ++i)
    {
      l_dirent->name[b++] = linebuffer[i];
    }

  // check file type
#define X(a,b,c) linebuffer[len-3]==a && linebuffer[len-2]==b && linebuffer[len-1]==c

  if (X('p','r','g'))
    {
      l_dirent->type = CBM_T_PRG;
    }
  else if (X('s','e','q'))
    {
      l_dirent->type = CBM_T_SEQ;
    }
  else if (X('u','s','r'))
    {
      l_dirent->type = CBM_T_USR;
    }
  else if (X('d','e','l'))
    {
      l_dirent->type = CBM_T_DEL;
    }
  else if (X('r','e','l'))
    {
      l_dirent->type = CBM_T_REL;
    }
  else if (X('c','b','m'))
    {
      l_dirent->type = CBM_T_CBM;
    }
  else if (X('d','i','r'))
    {
      l_dirent->type = CBM_T_DIR;
    }
  else if (X('v','r','p'))
    {
      l_dirent->type = CBM_T_VRP;
    }
  else if (X('l','n','k'))
    {
      l_dirent->type = CBM_T_LNK;
    }
  else
    {
      // parse header
      l_dirent->type = _CBM_T_HEADER;

      // skip one character which should be "
      if (linebuffer[i] == '"')
        {
          ++i;
        }
      // skip one character which should be space
      if (linebuffer[i] == ' ')
        {
          ++i;
        }

      // copy disk ID
      for(b = 0; i < sizeof(linebuffer) && b < DISK_ID_LEN; ++i, ++b)
        {
          disk_id_buf[b] = linebuffer[i];
        }

      // strip disk name
      for(b = 15; b > 0; --b)
        {
          if (l_dirent->name[b] == 0 ||
              l_dirent->name[b] == ' ' ||
              l_dirent->name[b] == 0xA0)
            {
              l_dirent->name[b] = 0;
              continue;
            }
          break;
        }

      return 0;
    }

  // parse read-only
  l_dirent->access = (linebuffer[i-4] == 0x3C) ? CBM_A_RO : CBM_A_RW;

  return 0;
}

unsigned char UCIReadDir(struct cbm_dirent* l_dirent) {
// Read dir entries on UCI file system
    unsigned char presenttype;
    unsigned char len;

    if(!uii_isdataavailable()) { return 1; }

    // Reset entry type
    presenttype = 0;

    // Get next dir entry
	uii_readdata();
	uii_accept();

    // Check if entry is a dir by checking if bit 4 of first byte is set
    if(uii_data[0]&0x10) { presenttype=CBM_T_DIR; }
    
    // Truncate entry at 16 chars
    uii_data[17]=0;

    // Copy to buffer and find out length
    strcpy(linebuffer,AscToPet(uii_data+1));
    len=strlen(linebuffer);

    // check file type
    #define X(a,b,c) linebuffer[len-3]==a && linebuffer[len-2]==b && linebuffer[len-1]==c

    if(!presenttype && len>4) {
        if (X('p','r','g') || X('P','R','G'))
        {
          presenttype = CBM_T_PRG;
        }
        else if (X('s','e','q') || X('S','E','Q'))
        {
          presenttype = CBM_T_SEQ;
        }
        else if (X('u','s','r') || X('U','S','R'))
        {
          presenttype = CBM_T_USR;
        }
        else if (X('d','e','l') || X('D','E','L'))
        {
          presenttype = CBM_T_DEL;
        }
        else if (X('r','e','l') || X('R','E','L'))
        {
          presenttype = CBM_T_REL;
        }
        else if (X('c','b','m') || X('C','B','M'))
        {
          presenttype = CBM_T_CBM;
        }
        else if (X('d','i','r') || X('D','I','R'))
        {
          presenttype = CBM_T_DIR;
        }
        else if (X('v','r','p') || X('V','R','P'))
        {
          presenttype = CBM_T_VRP;
        }
        else if (X('l','n','k') || X('L','N','K'))
        {
          presenttype = CBM_T_LNK;
        }
    }

    // Set SEQ as default for too long or unrecognised filenames
    if(!presenttype) { presenttype = CBM_T_SEQ; }

    // Set direntry data
    strcpy(l_dirent->name,linebuffer);
    l_dirent->type = presenttype;
    l_dirent->size = 0;
    return 0;
}

unsigned char readDir(const BYTE device, const BYTE sorted) {
/**
 * read directory of device @p device.
 * @param device CBM device number
 * @param sorted if true, return directory entries sorted.
 * @param context window context.
 * @return 1 on success
 */

    BYTE cnt = 0xff;
    BYTE ret;
    const BYTE y = 3;
    BYTE x = 0;
    BYTE inserted;
    struct DirElement *iterate;
    struct DirElement *p;

    previous = 0;

    freeDir();
    memset(&cwd,0,sizeof(cwd));
    memset(disk_id_buf, 0, DISK_ID_LEN);

    if(!fb_uci_mode) {
        if (cbm_opendir(device, device) != 0)
        {
            cbm_closedir(device);
            return NULL;
        }
        else {
            uii_open_dir();
            if(!uii_success()) {
                uii_abort();
                return NULL;
            }
            uii_get_dir();
        }
    }

    while(1)
    {
        inserted = 0;
        current = calloc(1,sizeof(direlement_size));
        if (!current) { goto stop; }

        if(!fb_uci_mode) {
            ret = myCbmReadDir(device, &(current->dirent));
        } else {
            ret = UCIReadDir(&(current->dirent));
        }
        if (ret != 0) {
            goto stop;
        }
    
        // print progress bar
        if ((cnt>>2) >= DIRW-4) {
            x = 4;
            revers(0);
            cnt = 0;
            gotoxy(x, y);
            cclear(DIRW-4);
            gotoxy(0, y);
            if(!fb_uci_mode) {
                cprintf("[%02i]", device);
            }
        } else {
            gotoxy(x + (cnt>>2), y);
            revers(progressRev[cnt & 3]);
            cputc(progressBar[cnt & 3]);
            ++cnt;
        }

        if (!cwd.name[0] && !fb_uci_mode)
        {
            // initialize directory
            if (current->dirent.type == _CBM_T_HEADER) {
                BYTE i;
                for(i = 0; current->dirent.name[i]; ++i) {
                    cwd.name[i] = current->dirent.name[i];
                }
                cwd.name[i++] = ',';
                memcpy(&cwd.name[i], disk_id_buf, DISK_ID_LEN);
            } else {
                strcpy(cwd.name, "Unknown type");
            }
            free(current);
        }
        else if (current->dirent.type==CBM_T_FREE)
        {
            // blocks free entry
            cwd.free=current->dirent.size;
            goto stop;
        }
        else if (cwd.firstelement==NULL)
        {
            // first element
            cwd.firstelement = current;
            previous = current;
        } else {
            // all other elements
            if (sorted) {
                // iterate the sorted list
                for(iterate = cwd.firstelement; iterate->next; iterate = iterate->next)
                {
                  // if the new name is greater than the current list item,
                  // it needs to be inserted in the previous position.
                  if (strncmp(iterate->dirent.name, current->dirent.name, 16) > 0)
                    {
                      // if the previous position is NULL, insert at the front of the list
                      if (! iterate->prev) {
                          current->next = iterate;
                          iterate->prev = current;
                          cwd.firstelement = current;
                      }
                      else {
                          // insert somewhere in the middle
                          p = iterate->prev;
                          p->next = current;
                          current->next = iterate;   
                          current->prev = p;
                          iterate->prev = current;
                      }
                      inserted = 1;
                    }
                }
                if(!inserted) {
                    iterate->next = current;
                    current->prev = iterate;
                }
            }
            else {
              current->prev = previous;
              previous->next = current;
              previous = current;
            }
        }
    }
    
    stop:
    if(!fb_uci_mode) {
        cbm_closedir(device);
    }
    revers(0);

    if (cwd.firstelement) {
        cwd.selected = cwd.firstelement;
        cwd.firstprinted = cwd.firstelement;
    }
    return 1;
}

// File browser operations functions
const char* fileTypeToStr(BYTE ft)
{
  if (ft & _CBM_T_REG)
    {
      ft &= ~_CBM_T_REG;
      if (ft <= 4)
        return reg_types[ft];
    }
  else
    {
      if (ft <= 5)
        return oth_types[ft];
    }
  bad_type[0] = '?';
  bad_type[1] = value2hex[ft >> 4];
  bad_type[2] = value2hex[ft & 15];
  bad_type[3] = 0;
  return bad_type;
}

void CheckMounttype(const char *dirname) {
  register BYTE l = strlen(dirname);

  mountflag = 0;
  
  if(dirname) {
      if (l > 4 && dirname[l-4] == '.')
      {
        if ((dirname[l-3] == 'd' || dirname[l-3] == 'D') &&
            (dirname[l-2] == '6') &&
            (dirname[l-1] == '4'))
          {
            mountflag = 1;
          }
        if ((dirname[l-3] == 'g' || dirname[l-3] == 'G') &&
          (dirname[l-2] == '6') &&
          (dirname[l-1] == '4'))
        {
          mountflag = 1;
        }
        else if ((dirname[l-3] == 'd' || dirname[l-3] == 'D') &&
                 (dirname[l-2] == '7' || dirname[l-2] == '8') &&
                 (dirname[l-1] == '1'))
          {
            mountflag = 1;
          }
        else if ((dirname[l-3] == 'g' || dirname[l-3] == 'G') &&
                 (dirname[l-2] == '7' || dirname[l-2] == '8') &&
                 (dirname[l-1] == '1'))
          {
            mountflag = 1;
          }
        else if ((dirname[l-3] == 'd' || dirname[l-3] == 'D') &&
                 (dirname[l-2] == 'n' || dirname[l-2] == 'N') &&
                 (dirname[l-1] == 'p' || dirname[l-1] == 'P'))
          {
            mountflag = 1;
          }
        else if ((dirname[l-3] == 'r' || dirname[l-3] == 'R') &&
                 (dirname[l-2] == 'e' || dirname[l-2] == 'E') &&
                 (dirname[l-1] == 'u' || dirname[l-1] == 'U'))
          {
            mountflag = 2;
          }
      }
  }
}

void drawDirFrame()
{
  unsigned char length;

  clearArea(0,3,DIRW,22);

  gotoxy(0,3);
  if(fb_uci_mode) {
    cprintf("[UCI file system]");
  } else {
    cprintf("[%02i] %s",device,cwd.name);
  }
  
  gotoxy(0,24);
  cprintf("(%s) %u blocks free",drivetype[devicetype[device]],cwd.free);

  if(trace || fb_uci_mode) {
    if(fb_uci_mode) {
        uii_get_path();
        strcpy(linebuffer,uii_data);
    } else {
        strcpy((char*)utilbuffer,pathconcat());
    }
    length = strlen((char*)utilbuffer);
    if(length > DIRW) {
      strcpy(linebuffer,(char*)utilbuffer+length-DIRW);
    } else {
      strcpy(linebuffer,(char*)utilbuffer);
    }
  }
  else {
    strcpy(linebuffer,"No dirtrace active.");
  }
  gotoxy(0,4);
  cputs(linebuffer);
}

void clrDir()
{
  clearArea(0,5,DIRW,19);
}

void printElementPriv(const BYTE xpos, const BYTE ypos)
{
    textcolor(DC_COLOR_HIGHLIGHT);
    gotoxy(xpos,ypos);
    if ((current == cwd.selected)) {
        textcolor(DMB_COLOR_SELECT);
        revers(1);
    }

    // if blocks are >= 10000 shorten the file type to 2 characters
    strcpy(linebuffer2, fileTypeToStr(current->dirent.type));
    if (current->dirent.size >= 10000 && strlen(current->dirent.name) == 16) {
        linebuffer2[0] = linebuffer2[1];
        linebuffer2[1] = linebuffer2[2];
        linebuffer2[2] = 0;
    }
    if(fb_uci_mode) {
        cprintf("%-16s %s",current->dirent.name,linebuffer2);
    } else {
        cprintf((current->dirent.size < 10000) ? "%4u %-16s %s" : "%u %-15s %s",
            current->dirent.size,
            current->dirent.name,
            linebuffer2);
    }
    
    if (current->flags!=0)
      {
        gotoxy(xpos,ypos);
        cputc('>');
      }
    textcolor(DC_COLOR_TEXT);
    revers(0);
}

void printDir() {
    int selidx = 0;
    int page = 0;
    int skip = 0;
    int pos = cwd.pos;
    BYTE lastpage = 0;
    int idx = 0;
    int xpos,ypos;
    int DIRH = (SCREENW==80)? 38:19;
    const char *typestr = NULL; 

    lastpage=pos/DIRH;

    if (!cwd.firstelement)
    {
      clrDir();
      return;
    }

    clrDir();
    revers(0);
    current = cwd.firstprinted;

    for(idx=0; (current != NULL) && (idx < DIRH); ++idx) {
        xpos = 0;
        ypos = idx+5;
        printElementPriv(xpos,ypos);
        current = current->next;
    }

    // clear empty lines
    for (;idx < DIRH; ++idx) {
        xpos = 1;
        ypos = idx+5;
        gotoxy(xpos,ypos);
        cputs("                         ");
    }
}

void showDir()
{
  drawDirFrame();
  printDir();
}

void refreshDir(const BYTE sorted)
{
  readDir(device, sorted);
  cwd.selected=cwd.firstelement;
  showDir();
}

int changeDir(const BYTE device, char *dirname, const BYTE sorted)
{
    int ret;
    register BYTE l = strlen(dirname);
  
    if (dirname) {
        CheckMounttype(dirname);

        if(mountflag==2 && trace == 1) {
          reuflag = 1;
          strcpy(imagename,dirname );
        }   
        if(fb_uci_mode) {   
            uii_change_dir(dirname);
        } else {
            if (mountflag==1 || (l == 1 && dirname[0]==CH_LARROW) || 
            devicetype[device] == VICE || devicetype[device] == U64) {
                sprintf(linebuffer, "cd:%s", dirname);
            }
            else
            {
                sprintf(linebuffer, "cd/%s/", dirname);
            }
        }

        if (trace == 1 )
        {
            strcpy(path[depth], dirname);
        }
    } else {
        if(fb_uci_mode) {
            uii_change_dir("/");
        } else {
            strcpy(linebuffer, "cd//");
        }
    }
    if(fb_uci_mode) {
        ret = (uii_success())?0:1;
    } else {
        ret = cmd(device, linebuffer);
    }
    if (ret == 0) { refreshDir(sorted); }
    return ret;
}

// Filebrowset interface functions
void updateMenu(void)
{
  BYTE menuy=2;

  clearArea(MENUX,3,15,22);
  revers(0);
  textcolor(DC_COLOR_TEXT);

  cputsxy(MENUX+1,++menuy," F1 Dir refr.");
  cputsxy(MENUX+1,++menuy," F3 UCI or IEC");
  if(!fb_uci_mode) {
    cputsxy(MENUX+1,++menuy,"+/- Device");
  }
  cputsxy(MENUX+1,++menuy,"RET Run/Select");
  cputsxy(MENUX+1,++menuy,"DEL Dir up");
  cputsxy(MENUX+1,++menuy,"  \x5e Root dir");
  cputsxy(MENUX+1,++menuy,"  T Top");
  cputsxy(MENUX+1,++menuy,"  E End");
  cputsxy(MENUX+1,++menuy,"  S Sort");
  cputsxy(MENUX+1,++menuy,"P/U Page up/do");
  cputsxy(MENUX+1,++menuy,"Cur Navigate");
  if(!fb_uci_mode) {
    cputsxy(MENUX+1,++menuy,"  D Dirtrace");
  }
  if(trace || fb_uci_mode) {
    cputsxy(MENUX+1,++menuy," AB Add mount");
    cputsxy(MENUX+1,++menuy,"  M Run mount");
  } else { menuy += 2; }
  cputsxy(MENUX+1,++menuy," F7 Quit");

  menuy++;
  if(fb_uci_mode) {
    cputsxy(MENUX,++menuy," UCI mode");
  } else {
    cputsxy(MENUX,++menuy," IEC mode");
  }
  if (trace == 1)
  {
    cputsxy(MENUX,++menuy," Trace   ON ");
  }
  else
  {
    cputsxy(MENUX,++menuy," Trace   OFF");
  }
}

void updateScreen()
{
  clrscr();
  headertext("Filebrowser");
  textcolor(DC_COLOR_TEXT);
  updateMenu();
  showDir();
}

void FindFirstIECDrive() {
    BYTE i;

    for(i = 0; i < 16; ++i) {
        cbm_close(i);
        cbm_closedir(i);
    }

    textcolor(DC_COLOR_TEXT);
    i = 7;

    while(++i < MAXDEVID+1) {
        device = i;
        if(readDir(device, sorted)) {
            getDeviceType(device);
            showDir();
            goto found_first_drive;
        }
    }

    found_first_drive:;
}

void mainLoopBrowse(void)
{
  unsigned int pos = 0;
  BYTE lastpage = 0;
  BYTE nextpage = 0;
  int DIRH = (SCREENW==80)? 38:19;
  int xpos,ypos,yoff;
  unsigned char count;
  
  trace = 0;
  depth = 0;
  reuflag = 0;
  addmountflag = 0;
  runmountflag = 0;
  mountflag = 0;
  fb_uci_mode = 1;
  fb_selection_made = 2;

  readDir(0,sorted);
  updateScreen();

  while(1)
    {
      current = cwd.selected;
      pos=cwd.pos;
      lastpage=pos/DIRH;
      yoff=pos-(lastpage*DIRH);
      xpos = 0;
      ypos = yoff+5;

      switch (cgetc())
        {
        case 's':
          sorted = ! sorted;
          // fallthrough
        case '1':
        case CH_F1:
          readDir(device, sorted);
          showDir();
          break;

        case CH_F3:
          fb_uci_mode = ! fb_uci_mode;
          updateMenu();
          if(!fb_uci_mode) {
            FindFirstIECDrive();
          }
          break;

        case '2':
        case CH_F2:
        case '+':
          if(!fb_uci_mode) {
            if (++device > MAXDEVID) { device=8; }
            if (! devicetype[device])
            {
              getDeviceType(device);
            }
            memset(&cwd,0,sizeof(cwd));
            showDir();
          }          
          break;
        
        case '-':
          if(!fb_uci_mode) {
            if (--device < 8) { device=MAXDEVID; }
            if (! devicetype[device])
              {
                getDeviceType(device);
              }
            memset(&cwd,0,sizeof(cwd));
            showDir();
          }
          break;

        case 't':
        case CH_HOME:
          cwd.selected=cwd.firstelement;
          cwd.pos=0;
          printDir();
          break;

        case 'd':
          if(!fb_uci_mode) {
            if (trace == 0)
            {
              trace = 1;
              pathdevice = device;
              changeDir(device, NULL, sorted);
            }
            else
            {
              trace = 0;
              depth = 0;
              showDir();
            }
            updateMenu();
          }
          break;

        case 'e':
          current = cwd.firstelement;
          pos=0;
          while (1)
            {
              if (current->next!=NULL)
                {
                  current=current->next;
                  pos++;
                }
              else
                {
                  break;
                }
            }
          cwd.selected=current;
          cwd.pos=pos;
          printDir();
          break;

        case 'q':
        case CH_F7:
          trace = 0;
          goto done;

        case CH_CURS_DOWN:
          if (cwd.selected!=NULL && current->next!=NULL)
            {
              current=current->next;
              cwd.selected=current;
              nextpage=(pos+1)/DIRH;
              cwd.pos++;
              if (lastpage!=nextpage)
                {
                  cwd.firstprinted = current;
                  printDir();
                }
              else
                {
                  current=current->prev;
                  printElementPriv(xpos, ypos);
                  yoff++;
                  xpos = 0;
                  ypos = yoff+5;
                  current=current->next;
                  printElementPriv(xpos, ypos);
                }
            }
          break;

        case CH_CURS_UP:
          if (cwd.selected!=NULL && current->prev!=NULL)
            {
              current=current->prev;
              cwd.selected=current;
              nextpage=(pos-1)/DIRH;
              cwd.pos--;
              if (lastpage!=nextpage)
                {
                  for(count=0;count<DIRH;count++) {
                    if(current->prev != NULL) {
                      current=current->prev;
                    }
                  }
                  cwd.firstprinted = current;
                  printDir();
                }
              else
                {
                  current=current->next;
                  printElementPriv(xpos, ypos);
                  yoff--;
                  xpos = 0;
                  ypos = yoff+5;
                  current=current->prev;
                  printElementPriv(xpos, ypos);
                }
            }
          break;

        // --- start / enter directory
        case CH_ENTER:
        case CH_CURS_RIGHT:
          // Executable PRG?
          if (cwd.selected && current->dirent.type==CBM_T_PRG)
            {
              if (trace == 0 && !fb_uci_mode)
              {
                execute(current->dirent.name,device, 0, "");
              }
              else
              {
                strcpy(pathfile, current->dirent.name );
                pathrunboot = 0;
                fb_selection_made = 1;
                goto done;
              }             
            }
          // else change dir
          if (cwd.selected)
            {
              if (trace == 1) {
                strcpy(path[depth++],current->dirent.name);
              }
              changeDir(device, current->dirent.name, sorted);
            }
            if(reuflag) {
                fb_selection_made=1;
                goto done;
            }
          break;

        // --- leave directory
        case CH_CURS_LEFT:
        case CH_DEL:
          if (trace == 1)
          {
            --depth;
          }
          if(fb_uci_mode) {
            changeDir(0, "..", sorted);
          } else {
            changeDir(device, devicetype[device] == U64?"..":"\xff", sorted);
          }
          break;

        // Page down
        case 'p':
        // Check if not already last item? If no, page down
          if(current->next!=NULL) {
              cwd.selected = 0;
              printElementPriv(xpos, ypos);
              for(count=0;count<DIRH;count++)
              {
                if(current->next) {
                  current=current->next;
                  cwd.pos++;
                  cwd.selected=current;
                  cwd.firstprinted=current;
                }
              }
              pos=cwd.pos;
              yoff=pos-(lastpage*DIRH);
              xpos = 0;
              ypos = yoff+5;
              printDir();
          }
          break;

        // Page up
        case 'u':
        // Check if not already first item? If no, page up
          if(current->prev!=NULL) {
              cwd.selected = 0;
              printElementPriv(xpos, ypos);
              for(count=0;count<DIRH;count++)
              {
                if(current->prev) {
                  current=current->prev;
                  cwd.pos--;
                  cwd.selected=current;
                  cwd.firstprinted=current;
                }
              }
              pos=cwd.pos;
              yoff=pos-(lastpage*DIRH);
              xpos = 0;
              ypos = yoff+5;
              printDir();
          }
          break;

        case CH_UARROW:
          if (trace == 1)
          {
            depth = 0;
          }
          changeDir(device, NULL, sorted);
          break;

        case 'a':
          CheckMounttype(current->dirent.name);
          if(mountflag==1) {
            addmountflag = 1;
            strcpy(imageaname,current->dirent.name);
            fb_selection_made=1;
            goto done;
          }
        
        case 'b':
          CheckMounttype(current->dirent.name);
          if(mountflag==1) {
            addmountflag = 2;
            strcpy(imagebname,current->dirent.name);
            fb_selection_made=1;
            goto done;
          }

        case 'm':
          if(mountflag==1 && imageaid) {
            runmountflag = 1;
            strcpy(pathfile, current->dirent.name );
            pathrunboot = 0;
            fb_selection_made=1;
            goto done;
          }
        }
    }

 done:;
    freeDir();
    clearArea(0,2,40,23);
    gotoxy(0,3);
    cputs("Reading slots back to memory.");
    bankrun(0);
}
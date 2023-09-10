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
#include "core.h"
#include "u-time.h"
#include "ultimate_common_lib.h"
#include "ultimate_dos_lib.h"
#include "ultimate_time_lib.h"
#include "ultimate_network_lib.h"
#include "fc3.h"

#pragma code-name	("CODE2");
#pragma rodata-name	("RODATA2");

// Config file I/O functions
void CheckStatus(char* message) {
// Function to check UII+ status and print error box if applicable

    if (!uii_success()) {
        printf("\n\rI/O error in %s.\n\r",message);
        printf("\n\rStatus: %s\n\r",uii_status);
        uii_abort();
        errorexit();
    }
}

void FreeSlotMemory() {
  free((void*)slotaddress_start); 
  slotaddress_start=0;
}

void std_write(char * filename,unsigned char verbose)
{
    unsigned char x;

    // Delete old config file as I can not (yet) get overwrite to work
	  uii_delete_file(filename);

    // Save slots via UCI, one slot at a time due to 512 byte limit
    uii_open_file(0x06,filename);
    
    for(x=0; x<18; ++x)
    {
      if(verbose) {
        gotoxy(0,8);
        cprintf("Writing slot %2d ",x+1);
      }
      uii_write_file((void*)(slotaddress_start + (x*sizeof(Slot))),sizeof(Slot));
      CheckStatus("writing slots");
    }

    uii_close_file();
}

void std_read(char * filename,unsigned char verbose)
{
    // Function to read config file
    // Input: file_name is the name of the config file

    unsigned char x;

    // Allocate slots memory
    if(slotaddress_start) { FreeSlotMemory(); }
    slotaddress_start = (unsigned int) calloc(18,sizeof(Slot));

    // Abort if insufficient memory
    if(!slotaddress_start) {
        printf("\n\rOut of memory.\n\r");
        errorexit();
    }

    uii_open_file(0x01,filename);

    // Check if a file already exists, otherwise create new one
    if(strcmp((const char*)uii_status,"00,ok") != 0)
    {
        for(x=0; x<18; ++x)
        {
            if(verbose) {
              gotoxy(0,8);
              cprintf("Creating slot %2d",x+1);
            }
            strcpy(Slot.menu,"");
            strcpy(Slot.path,"");
            strcpy(Slot.file,"");
            strcpy(Slot.cmd,"");
            strcpy(Slot.reu_image,"");
            Slot.device = 0;
            Slot.runboot = 0;
            Slot.command = 0;
            Slot.cfgvs = CFGVERSION;
            strcpy(Slot.image_a_path,"");
            strcpy(Slot.image_a_file,"");
            Slot.image_a_id = 0;
            strcpy(Slot.image_b_path,"");
            strcpy(Slot.image_b_file,"");
            Slot.image_b_id = 0;
            memcpy((void*)(slotaddress_start + (x*sizeof(Slot))),&Slot,sizeof(Slot));
        }
        std_write(filename,1);
        uii_close_file();
        return;
    }

    uii_read_file(sizeof(Slot));
    for(x=0; x<18; ++x)
    {
      if(verbose) {
        gotoxy(0,8);
        cprintf("Reading slot %2d",x+1);
      }
      uii_readdata();
      uii_accept();
      CheckStatus("reading slots");
      memcpy((void*)(slotaddress_start + (x*sizeof(Slot))),&uii_data,sizeof(Slot));
    }
    
    uii_close_file();

    memcpy(&Slot,(void*)slotaddress_start,sizeof(Slot));
    if(Slot.cfgvs < CFGVERSION) {
      printf("\n\rOld configuration file format.");
      printf("\n\rRun upgrade tool first.");
      printf("\n\rPress key to exit.\n\r");
      errorexit();
    }
}

void writeconfigfile(char* filename)
{
	// Function to write config file
	// Inout: filename of config file

  unsigned char x;

  // Clear buffer memory
  memset(utilbuffer,0,328);

	// Place all variables in buffer memory
  for(x=0;x<60;x++)
  {
    utilbuffer[x]=reufilepath[x];
  }

  for(x=0;x<20;x++)
  {
    utilbuffer[x+60]=imagename[x];
  }
  for(x=0;x<60;x++)
  {
    utilbuffer[x+80]=imageapath[x];
  }
  for(x=0;x<20;x++)
  {
    utilbuffer[x+140]=imageaname[x];
  }
  for(x=0;x<60;x++)
  {
    utilbuffer[x+160]=imagebpath[x];
  }
  for(x=0;x<20;x++)
  {
    utilbuffer[x+220]=imagebname[x];
  }
  utilbuffer[240] = reusize;
  utilbuffer[241] = timeonflag;

  utilbuffer[242] = (secondsfromutc & 0xFF000000) >> 24;
  utilbuffer[243] = (secondsfromutc & 0xFF0000) >> 16;
  utilbuffer[244] = (secondsfromutc & 0xFF00) >> 8;
  utilbuffer[245] = secondsfromutc & 0xFF;

  utilbuffer[246] = imageaid;
  utilbuffer[247] = imagebid;
  for(x=0;x<80;x++)
  {
    utilbuffer[248+x] = host[x];
  }
  
  // Delete old config file as I can not (yet) get overwrite to work
	uii_delete_file(filename);
  // Uncomment for debbug
  //printf("\nStatus: %s", uii_status);

	uii_open_file(0x06,filename);
  // Uncomment for debbug
  //printf("\nStatus: %s", uii_status);

	uii_write_file(utilbuffer,328);
  CheckStatus("writing config");
  // Uncomment for debbug
  //printf("\nStatus: %s", uii_status);

	uii_close_file();
  // Uncomment for debbug
  //printf("\nStatus: %s", uii_status);
}

void readconfigfile(char* filename)
{
	// Function to read config file
	// Inout: filename of config file

  unsigned char x;

	uii_open_file(0x01,filename);
  // Uncomment for debbug
  //printf("\nStatus: %s", uii_status);

  // Write a config file with default values if no file is found
  if(strcmp((const char*)uii_status,"00,ok") != 0)
  {
    printf("\nNo config file found, writing defaults.");
    writeconfigfile(filename);
    return;
  }

	uii_read_file(328);
  CheckStatus("reading config");
  // Uncomment for debbug
  //printf("\nStatus: %s", uii_status);

	uii_readdata();
  // Uncomment for debbug
  //printf("\nStatus: %s", uii_status);

	uii_accept();
  // Uncomment for debbug
  //printf("\nStatus: %s", uii_status);

  // Read variables from read data
  for(x=0;x<60;x++)
  {
    reufilepath[x]=uii_data[x];
  }
  for(x=0;x<20;x++)
  {
    imagename[x]=uii_data[x+60];
  }
  for(x=0;x<60;x++)
  {
    imageapath[x]=uii_data[x+80];
  }
  for(x=0;x<20;x++)
  {
    imageaname[x]=uii_data[x+140];
  }
  for(x=0;x<60;x++)
  {
    imagebpath[x]=uii_data[x+160];
  }
  for(x=0;x<20;x++)
  {
    imagebname[x]=uii_data[x+220];
  }
  
  reusize = uii_data[240];

  timeonflag = uii_data[241];
  
  secondsfromutc = uii_data[245] | (((unsigned long)uii_data[244])<<8)| (((unsigned long)uii_data[243])<<16)| (((unsigned long)uii_data[242])<<24);

  imageaid = uii_data[246];
  imagebid = uii_data[247];

  for(x=0;x<80;x++)
  {
    host[x] = uii_data[248+x];
  }

  // If no hostname is read due to old config file format, set default
  if(strlen(host)==0) { strcpy(host,"pool.ntp.org"); }

	uii_close_file();
  // Uncomment for debbug
  //printf("\nStatus: %s", uii_status);
	
  // Debug messages. Uncomment for debug mode
  //printf("\nREU file path+name: %s%s", reufilepath, imagename);
  //printf("\nImage a ID: %i",imageaid);
  //printf("\nImage b ID: %i",imagebid);
  //printf("\nImage a path+name: %s%s", imageapath, imageaname);
  //printf("\nImage b path+name: %s%s", imagebpath, imagebname);
  //printf("\nREU size: %i", reusize);
  //printf("\nTime on flag: %i", timeonflag);
  //printf("\nConverted UTC offset: %ld",secondsfromutc);
  //printf("\nNTP Hostname: %s",host);
  //cgetc();
}
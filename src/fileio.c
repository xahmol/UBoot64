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
  free(FirstSlot); 
  FirstSlot=0;
}

void std_write(char * filename,unsigned char verbose)
{
    unsigned char x;

    // Delete old config file as I can not (yet) get overwrite to work
	  uii_delete_file(filename);

    // Save slots via UCI, one slot at a time due to 512 byte limit
    uii_open_file(0x06,filename);

    Slot = FirstSlot;
    
    for(x=0; x<18; ++x)
    {
      if(verbose) {
        gotoxy(0,8);
        cprintf("Writing slot %2d  ",x+1,Slot);
      }
      uii_write_file((unsigned char*)Slot,SLOTSIZE);
      CheckStatus("writing slots");
      Slot++;
    }

    uii_close_file();
}

void std_read(char * filename,unsigned char verbose)
{
    // Function to read config file
    // Input: file_name is the name of the config file

    unsigned char x;
    char* readaddress;

    // Allocate slots memory
    if(FirstSlot) { FreeSlotMemory(); }
    FirstSlot = calloc(19,SLOTSIZE);

    // Abort if insufficient memory
    if(!FirstSlot) {
        printf("\n\rOut of memory.\n\r");
        errorexit();
    }

    Slot = FirstSlot;
    readaddress = (char*)FirstSlot;

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
            sprintf(Slot->menu,"Debug %2d",x);
            //strcpy(Slot->menu,"");
            strcpy(Slot->path,"");
            strcpy(Slot->file,"");
            strcpy(Slot->cmd,"");
            strcpy(Slot->reu_image,"");
            Slot->device = 0;
            Slot->runboot = 0;
            Slot->command = 0;
            Slot->cfgvs = CFGVERSION;
            strcpy(Slot->image_a_path,"");
            strcpy(Slot->image_a_file,"");
            Slot->image_a_id = 0;
            strcpy(Slot->image_b_path,"");
            strcpy(Slot->image_b_file,"");
            Slot->image_b_id = 0;
            Slot++;
        }
        std_write(filename,1);
        uii_close_file();
        return;
    }

    uii_read_file(SLOTSIZE*18);

    while (uii_isdataavailable()) {
      if(verbose) {
        gotoxy(0,9);
        cprintf("Reading slots: %4X",readaddress);
      }

      while (uii_isdataavailable()) {
        POKE(readaddress++,*respdatareg);       
      }
      uii_accept();
      CheckStatus("reading slots");
    }
    
    uii_close_file();

    Slot = FirstSlot;
    if(Slot->cfgvs < CFGVERSION) {
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
  memset(utilbuffer,0,sizeof(utilbuffer));

	// Place all variables in buffer memory
  utilbuffer[0] = CFGVERSION;
  utilbuffer[1] = timeonflag;
  utilbuffer[2] = (secondsfromutc & 0xFF000000) >> 24;
  utilbuffer[3] = (secondsfromutc & 0xFF0000) >> 16;
  utilbuffer[4] = (secondsfromutc & 0xFF00) >> 8;
  utilbuffer[5] = secondsfromutc & 0xFF;

  for(x=0;x<80;x++)
  {
    utilbuffer[6+x] = host[x];
  }
  
  // Delete old config file as I can not (yet) get overwrite to work
	uii_delete_file(filename);
	uii_open_file(0x06,filename);
	uii_write_file(utilbuffer,sizeof(utilbuffer));
  CheckStatus("writing config");
	uii_close_file();
}

void readconfigfile(char* filename)
{
	// Function to read config file
	// Inout: filename of config file

  unsigned char x;

	uii_open_file(0x01,filename);

  // Write a config file with default values if no file is found
  if(strcmp((const char*)uii_status,"00,ok") != 0)
  {
    printf("\nNo config file found, writing defaults.");
    writeconfigfile(filename);
    return;
  }

	uii_read_file(sizeof(utilbuffer));
  CheckStatus("reading config");
	uii_readdata();
	uii_accept();


  // Read variables from read data
  timeonflag = uii_data[1];
  
  secondsfromutc = uii_data[5] | (((unsigned long)uii_data[4])<<8)| (((unsigned long)uii_data[3])<<16)| (((unsigned long)uii_data[2])<<24);

  for(x=0;x<80;x++)
  {
    host[x] = uii_data[6+x];
  }

  // If no hostname is read due to old config file format, set default
  if(strlen(host)==0) { strcpy(host,"pool.ntp.org"); }

	uii_close_file();
}
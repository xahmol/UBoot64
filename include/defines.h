#ifndef DEFINES_H_
#define DEFINES_H_

// Color scheme
#define DC_COLOR_BG COLOR_BLACK
#define DC_COLOR_BORDER COLOR_BLACK
#define DC_COLOR_TEXT COLOR_YELLOW
#define DC_COLOR_HIGHLIGHT COLOR_WHITE
#define DC_COLOR_DIM COLOR_GREEN
#define DC_COLOR_ERROR COLOR_RED
#define DC_COLOR_WARNING COLOR_VIOLET
#define DC_COLOR_EE COLOR_LIGHTBLUE
#define DC_COLOR_GRAY COLOR_GRAY2
#define DC_COLOR_GRAYBRIGHT COLOR_GRAY3
#define DC_COLOR_WAITKEY COLOR_GREEN
#define DMB_COLOR_SELECT COLOR_CYAN
#define DMB_COLOR_HEADER1 COLOR_GREEN
#define DMB_COLOR_HEADER2 COLOR_LIGHTGREEN

// Command flag values
#define COMMAND_CMD             0x01
#define COMMAND_REU             0x02
#define COMMAND_IMGA            0x04
#define COMMAND_IMGB            0x08

// Execute flag values
#define EXEC_MOUNT              0x01
#define EXEC_COMMA1             0x02

// Config version
#define CFGVERSION              0x01

typedef unsigned char BYTE;

#define OK 0
#define ERROR -1
#define ABORT +1

// height of sceen
#define SCREENH 25
// bottom row on screen
#define BOTTOM (SCREENH-1)

// height of menu frame
#define MENUH SCREENH
// y position of menu frame
#define MENUY 0

// Define highest device ID allowed
#define MAXDEVID 30

// Spite control addresses
#define VIC_SPR2_X      0xD004
#define VIC_SPR2_Y      0xD005
#define VIC_SPR_HI_X    0xD010
#define VIC_SPR_ENA     0xD015
#define VIC_SPR2_COLOR  0xD029

// Standard location of config files on UCI filesystem
#define UCI_CFG_LOC     "/usb*/"

// Global variables
extern BYTE SCREENW;
extern BYTE DIRW;
extern BYTE MENUX; 
extern char path[8][20];
extern char pathfile[20];
extern BYTE pathdevice;
extern BYTE pathrunboot;
extern BYTE depth;
extern BYTE trace;
extern BYTE comma1;
extern BYTE commandflag;
extern BYTE mountflag;
extern BYTE reuflag;
extern BYTE addmountflag;
extern BYTE runmountflag;
struct SlotStruct {
    char path[100];
    char menu[21];
    char file[20];
    char cmd[80];
    char reu_image[20];
    BYTE reusize;
    BYTE runboot;
    BYTE device;
    BYTE command;
    BYTE cfgvs;
    char image_a_path[100];
    char image_a_file[20];
    BYTE image_a_id;
    char image_b_path[100];
    char image_b_file[20];
    BYTE image_b_id;
};
extern struct SlotStruct* Slot;
extern struct SlotStruct* FirstSlot;
extern struct SlotStruct* BufferSlot;
extern long secondsfromutc; 
extern unsigned char timeonflag;
extern char host[80];
extern char imagename[20];
extern char reufilepath[100];
extern char imageaname[20];
extern char imageapath[100];
extern unsigned char imageaid;
extern char imagebname[20];
extern char imagebpath[100];
extern unsigned char imagebid;
extern unsigned char reusize;
extern char* reusizelist[8];
extern unsigned char utilbuffer[86];
extern char configfilename[11];
extern char slotfilename[11];
extern unsigned char configversion;
extern unsigned char menuselect;
extern unsigned char fb_selection_made;
extern unsigned char fb_uci_mode;
extern unsigned char inside_mount;

// Import from cartridge back to BASIC code
extern char execute_commands[200];
extern char execute_keys[10];

#define SLOTSIZE    488

#define CH_LARROW   0x5f
#define CH_UARROW   0x5e
#define CH_POUND    0x5c

#define BUILD_YEAR_CH0 (__DATE__[ 7])
#define BUILD_YEAR_CH1 (__DATE__[ 8])
#define BUILD_YEAR_CH2 (__DATE__[ 9])
#define BUILD_YEAR_CH3 (__DATE__[10])


#define BUILD_MONTH_IS_JAN (__DATE__[0] == 'J' && __DATE__[1] == 'a' && __DATE__[2] == 'n')
#define BUILD_MONTH_IS_FEB (__DATE__[0] == 'F')
#define BUILD_MONTH_IS_MAR (__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'r')
#define BUILD_MONTH_IS_APR (__DATE__[0] == 'A' && __DATE__[1] == 'p')
#define BUILD_MONTH_IS_MAY (__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'y')
#define BUILD_MONTH_IS_JUN (__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'n')
#define BUILD_MONTH_IS_JUL (__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'l')
#define BUILD_MONTH_IS_AUG (__DATE__[0] == 'A' && __DATE__[1] == 'u')
#define BUILD_MONTH_IS_SEP (__DATE__[0] == 'S')
#define BUILD_MONTH_IS_OCT (__DATE__[0] == 'O')
#define BUILD_MONTH_IS_NOV (__DATE__[0] == 'N')
#define BUILD_MONTH_IS_DEC (__DATE__[0] == 'D')

#ifndef min
#define min(a,b) ( (a) < (b) ? (a) : (b) )
#endif

#define BUILD_MONTH_CH0 \
    ((BUILD_MONTH_IS_OCT || BUILD_MONTH_IS_NOV || BUILD_MONTH_IS_DEC) ? '1' : '0')

#define BUILD_MONTH_CH1 \
    ( \
        (BUILD_MONTH_IS_JAN) ? '1' : \
        (BUILD_MONTH_IS_FEB) ? '2' : \
        (BUILD_MONTH_IS_MAR) ? '3' : \
        (BUILD_MONTH_IS_APR) ? '4' : \
        (BUILD_MONTH_IS_MAY) ? '5' : \
        (BUILD_MONTH_IS_JUN) ? '6' : \
        (BUILD_MONTH_IS_JUL) ? '7' : \
        (BUILD_MONTH_IS_AUG) ? '8' : \
        (BUILD_MONTH_IS_SEP) ? '9' : \
        (BUILD_MONTH_IS_OCT) ? '0' : \
        (BUILD_MONTH_IS_NOV) ? '1' : \
        (BUILD_MONTH_IS_DEC) ? '2' : \
        /* error default */    '?' \
    )

#define BUILD_DAY_CH0 ((__DATE__[4] >= '0') ? (__DATE__[4]) : '0')
#define BUILD_DAY_CH1 (__DATE__[ 5])

#define BUILD_HOUR_CH0 (__TIME__[0])
#define BUILD_HOUR_CH1 (__TIME__[1])

#define BUILD_MIN_CH0 (__TIME__[3])
#define BUILD_MIN_CH1 (__TIME__[4])

#endif

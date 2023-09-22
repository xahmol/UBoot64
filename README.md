# UBoot64

![Logo](https://github.com/xahmol/UBoot64/blob/main/Screenshots/UBoot64%20-%20Logo.png?raw=true)

Boot menu for C64 Ultimate enabled devices

## Contents

[Version history and download](#version-history-and-download)

[Instructions](#instructions)

- [Prerequisites](#prerequisites)

- [Installation](#installation)

[Credits](#credits)

![Startup](https://github.com/xahmol/UBoot64/blob/main/Screenshots/UBoot64%20-%20Startup.png?raw=true)

![Main menu](https://github.com/xahmol/UBoot64/blob/main/Screenshots/UBoot64%20-%20Menu.png?raw=true)

## Version history and download
([Back to contents](#contents))

[Link to latest build](https://github.com/xahmol/UBoot64/raw/main/UBoot64-v091-20230922-1818.zip)

Version 0.91 - 20230922-1818:

- First public alpha

## Instructions

### Prerequisites

* UltimateII+ (U2+) cartridge installed on a real C64, or an Ultimate 64.
- Firmware at version 3.4 or higher (to have access to the UCI DRVINFO command, link to firmware page, scroll down for U2 firmware: <https://ultimate64.com/Firmware> )

### Installation

* Download the ZIP file with the latest build

* In the ZIP file you will find these instructions and a .crt file, the latter is the cartridge image containing the UBoot64 software. Unzip contents to a temporary location.

* Transfer this .crt file to the /Flash/Carts directory on your Ultimate device. Either transfer to this directory on your Ultimate device via FTP, or place it somewhere on the USB storage Ultimate device, browse to it in the UI filebrowser, press C= + C to copy, navigate to the /Flash/Carts dir and paste by C= + V/.

![Ultimate UI](https://github.com/xahmol/UBoot64/blob/main/Screenshots/UBooy64%20-%20Ultimate%20UI.png?raw=true)

![Ultimate Flash dir](https://github.com/xahmol/UBoot64/blob/main/Screenshots/UBoot64%20-%20Flashdir.png?raw=true)

![Ultimate Carts dir](https://github.com/xahmol/UBoot64/blob/main/Screenshots/UBoot64%20-%20carts%20dir.png?raw=true)

- Select this .crt file as cartridge to start automatically on start of your Ultimate device by, in the Ultimate UI, first press **F2**, navigate with cursor down to **C64 and Cartridge Settings**. press **RETURN**, press **RETURN** again to edit the **Cartridge** option, and select **uboot64.crt** from the drop down box. Press **STOP** to leave the configuration menu and press **RETURN** to confirm storing the settings to flash.

![Ultimate UI cart settings](https://github.com/xahmol/UBoot64/blob/main/Screenshots/UBoot64%20-%20UI%20cart%20settings.png?raw=true)

![Ultimate UI select cart](https://github.com/xahmol/UBoot64/blob/main/Screenshots/UBoot64%20-%20UI%20set%20cart.png?raw=true)

- Do a power cycke of your device. The UBoot64 software should now automatically start on starting your Ultimate device.

- Stop autostarting of UBoot64 by deselecting the cartridge via the same procedure and selecting **None** or another image as cartridge.

**First run:**

* At first run no configuration file is present yet for the menu, so only the default menu options are visible:
![](https://github.com/xahmol/DMBoot/raw/main/pictures/dmboot%20-%20firstrunmenu.png)
* For instructions of the menu options: see below.

**Add start options via the Filebrowser**

* Start options can be added to menuslots 0-9 and A-Z via the Filebrowser. This can be either running an executable program, or booting a specific disk image.
* For this, first choose **F1** for filebrowser.  You will get a screen like this:

![](https://github.com/xahmol/DMBoot/raw/main/pictures/dmboot%20-%20filebrowsermenu.png)

* Full instructions for the filebrowser are below. Here only the quick instructions to add an option to the startmenu.
* Select your desired drive target by switching pressing **+** or **-** to increase resp. decrease the device number until the desired device number is selected.
* Refresh directory by **F1** if needed (empty column)
* Start a directory trace by pressing **D**
  

![](https://github.com/xahmol/DMBoot/raw/main/pictures/dmboot%20-%20highlightdirtrace.png)

  This starts a trace of your movements through the directory tree, starting from the root directory of your device. You should see the directory refreshing to this root directory.
  You should also see the TRACE toggle switched to ON in the lower right corner of the screen.

![](https://github.com/xahmol/DMBoot/raw/main/pictures/dmboot%20-%20toggledirtraceon.png).

* Also note the other two toggles Frc 8 and FAST: these are toggled by pressing the **8** and **F** keys.

  ![](https://github.com/xahmol/DMBoot/raw/main/pictures/dmboot%20-%20highlightforce8andfast.png)

* Force 8: This forces the device ID to be loaded from to 8 regardless of the device ID the target is located at. Only works for the U2+ Software IEC drive (identified as U64 in the lowest line of the directory column). This enables loading software that is not supporting other parts of the program from other device IDs as 8
* FAST: this starts the program in FAST mode. Only use if the target is started in 80 column mode and the target supports it.
* Browse to your desired target via the **Cursor keys**: **UP/DOWN/LEFT?RIGHT** to move within the directory, **P** for page down and **U** for a page up, **ENTER** on a directory or disk image to enter the selected directory or image, **DEL** to change to parent directory.
* To choose to have a program executed from the menuslot, select **ENTER** or **F7** on the desired executable
* To have the program executed in C64 mode from the menuslot, select **6** on the desired executable

![](https://github.com/xahmol/DMBoot/blob/main/pictures/dmboot%20-%20filebrowsermenu_run64.png?raw=true)

* To choose to boot the image you are now in from the menuslot, select **F5**.
* To select a highlighted disk image (so a directory entry with an .dXX extension), press **A** or **B** to add this image to a menuslot as to be mounted on starting from that slot on the Ultimate emulated drive A resp. B. Then select the target menuslot, edit the name if desired, and enter the device ID for the image.

![](https://github.com/xahmol/DMBoot/blob/main/pictures/dmboot%20-%20filebrowsermenu_addmount.png?raw=true)

* To select the highligjhted program file to be started from the disk image mounted in drive A instead of from the hyperspeed drive, select the program by pressing **M** (instead of ENTER). Then select the target menuslot, edit the name if desired.

![](https://github.com/xahmol/DMBoot/blob/main/pictures/dmboot%20-%20filebrowsermenu_runmount.png?raw=true)

* To add a REU file to be loaded at the start of a menu option: navigate to the desired REU file and press 'ENTER'.
Then select the target menuslot, edit the name if desired, choose the REU size via '+' and '-' and press 'ENTER' to confirm.
**NOTE**: The REU file needs to be in the same filepath as the drive image for disk A if that one is present (not storage space available for another path to store).

* No validation if the configuration is correct or coherent is done, so setting up valid configurations is the users responsibility, Only very limited error handling on executing incorrectly configured menu slots is done. Mounts and REU file can be added or changed (by adding again and overwriting the previous one), but not deleted separately. To do so, the whole entry needs to be deleted. Adding disk images to mount, a REU file and choosing the file to start require all separate dirtrace actions in the filebrowser, so setting up a menuslot might take up to four entries to the filebrowser and navigating via dirtrace.

* After selecting what should be placed in a menuslot option, you should get this screen to select the menuslot position:

![](https://github.com/xahmol/DMBoot/raw/main/pictures/dmboot%20-%20choosemenuslot.png)

* If this slot is already (partly) filled, confirmation is asked. Choose Yes to proceed or No to Cancel.

![](https://github.com/xahmol/DMBoot/blob/main/pictures/dmboot%20-%20choosemenuslot-notempty.png?raw=true)

* Choose **0-9** or **A-Z** key to choose the desired slot.
* Enter the desired name for the menuslot and press **ENTER**
* You now return to the main menu where you should see the menu option appearing.
* Repeat until you have selected all desired menuslot options

**F1: Filebrowse menu**

The filebrowser based and insprired by the DraBrowse program from <https://github.com/doj/dracopy>
![](https://github.com/xahmol/DMBoot/raw/main/pictures/dmboot%20-%20filebrowsermenu.png)

Menu options are similar, but have diverged in newer DMBoot versions.

| Key            | Function                                                     |
| ---- | ------------------------------------------- |
| **F1 / 1**  | Read directory in current window |
| **+** | Increase devicenumber for the current window |
| **-** | Decrease devicenumber for the current window |
| **F5 / 5** | Boot from present directory / image (*added compared to DraBrowse*) |
| **RETURN** | Enter directory or run the selected program |
| **DEL** | Go to parent directory |
| **↑** | Go to root directory |
| **T** | Go to the first item in the directory |
| **E** | Go to the last item in the directory |
| **S** | Show directory entries sorted. WARNING: gets very slow on large dirs |
| **P** | Go a page down in the directory |
| **E** | Go a page up in the directory |
| **Cursor keys** | Navigate in the directory |
| **D** | Toggle Dirtrace: traces the directory movements from root directory to select menuslot option |
| **A** | Select the highlighted item as image to be mounted in drive A. Only works in Dirtrace mode, highlighted item should be an image with a .Dxx extention |
| **B** | Select the highlighted item as image to be mounted in drive A. Only works in Dirtrace mode, highlighted item should be an image with a .Dxx extention |
| **M** | Select the highlighted program to be started from the disk image mounted in drive A (instead of present device of dir) |
| **8** | Toggle Force 8, forcing device ID 8 on program execution or boot. Works for menuslot option as well as directly from filebrowser. |
| **F** | Toggle FAST, enabling this makes file execution or boot start in FAST mode. Works for menuslot option as well as directly from filebrowser. |
| **Q** | Quit to main menu |

**F2: Information**

Shows information screen. Also shouws how much VDC memory is detected. Press key to return to main menu.

![](https://github.com/xahmol/DMBoot/raw/main/pictures/dmboot%20-%20information%20screen.png)

**F3: Quit to C128 Basic**

Exit the bootmenu to the C128 BASIC Ready prompt. Memory will be erased on exit, SLOW mode will be selected also in 80 column mode for compatibility purposes.

**F4: NTP time / GEOS config**

Enables editing the settings for:
* automatically obtaining the actual time from an NTP server and setting the internal clock of the Ultimate II+ to this time
* RAM booting GEOS from a specified REU file with specified images mounted on the UII+ drives.

After pressing F4, you arrive at this screen:

![](https://github.com/xahmol/DMBoot/blob/main/pictures/dmboot%20-%20utilsettings.png?raw=true)

The screens shows you the present settings and allows you to edit them.

* **F1** Edit the settings for obtaining the time from an NTP server. This gives you this screen:

![](https://github.com/xahmol/DMBoot/blob/main/pictures/dmboot%20-%20ntpsettings.png?raw=true)

  * **F1** Toggles to enable or disable updating UII+ time from an NTP server at boot. Default: Enabled.
  * **F3** Edits the time offset to UTC (Universal standard time). The offset needs to provided in seconds. Automated adjustment for daylight savings ('Summer' and 'Winter' time) is not provided, so you have to adjust your offset on the change from daylight saving time to not. Example: Central Europen Time requires an offset of 3600, Central European Summer Time of 7200. See https://www.timeanddate.com/time/zones/ for all offsets in hours (multiply by 3600 to get to seconds). Default: 0 (=UTC).
  * **F5** Edits the NTP time server to be used. It defaults on pool.ntp.org, but you can specify your own if you want.
  * **F7** Back to main menu

* **F3** Edit the GEOS RAM boot configuration options. This gives you this screen:

![](https://github.com/xahmol/DMBoot/blob/main/pictures/dmboot%20-%20geossettings.png?raw=true)

  * **F1** Enables editing of the path to and the filename of the REU file to use on your UII+ device. Gives this screen:

![](https://github.com/xahmol/DMBoot/blob/main/pictures/dmboot%20-%20geosreupath.png?raw=true)

  * **F3** Enables editing the REU file size to match the image you have chosen. Choose **+** to increase size, **-** to decrease.

  ![](https://github.com/xahmol/DMBoot/blob/main/pictures/dmboot%20-%20geosreusize.png?raw=true)

  * **F5** Edits the device IDs, paths and the filenames of the images you want to mount.

  ![](https://github.com/xahmol/DMBoot/blob/main/pictures/dmboot%20-%20geosimages.png?raw=true)

  * **F7** Back to main menu

* **F7** Quit configuration tool. Only at this time new settings will be saved.

**F5: C64 mode**

Go to C64 mode (no confirmation will be asked)

**F6: GEOS RAM boot**

Boot GEOS from RAM with the specified settings. Those settings need to be configured first (via the F4 option in the main menu) for this to work.

Demonstration of booting GEOS via DMBoot (click picture to see video on Youtube):

[![](https://img.youtube.com/vi/u9hQ0eEtpeI/0.jpg)](https://www.youtube.com/watch?v=u9hQ0eEtpeI)

**F7: Edit / re-order / delete**

Enables to rename menuslots, re-order the slots or delete a slot. Selecting provides this menu:
![](https://github.com/xahmol/DMBoot/raw/main/pictures/dmboot%20-%20editreorderdelete.png)

* **F1** enables renaming a menuslot. Choose slot to be renamed by pressing **0-9** or **A-Z**. Enter new name. Enter to confirm.

![](https://github.com/xahmol/DMBoot/raw/main/pictures/dmboot%20-%20rename.png)

* **F2** enables to add a user defined command. This gives this screen:

![](https://github.com/xahmol/DMBoot/blob/main/pictures/dmboot%20-%20commandmountselect.png?raw=true)

**F1** will then enable to add an image to be mounted before the slot executable is started. Enter device ID (8 or 9 for the emulated drives, or 0 to cancel), path on the USB stick to the image and the image name.

**F2** will enable to add a user defined command to be executed before the program in the slot is started. Can be any valid BASIC command.

* **F3** enables re-ordering menu slots. Choose slot to be re-ordered by pressing **0-9** or **A-Z**. Selected menu slot is highlighted white. Move option by pressing **UP** or **DOWN**. Confirm by **ENTER**. Cancel with **F7**.

![](https://github.com/xahmol/DMBoot/raw/main/pictures/dmboot%20-%20reorder.png)

* **F5** enables deleting a menu slot. Choose slot to be re-ordered by pressing **0-9** or **A_Z**. Confirm by pressing **Y** for yes, or **N** for no.

![](https://github.com/xahmol/DMBoot/raw/main/pictures/dmboot%20-%20delete.png)

* **F7** takes you back to main menu. Changes made are saved only now.

## Credits

### UBoot64

Boot menu for C64 Ultimate enabled devices

Written in 2023 by Xander Mol
https://github.com/xahmol/UBoot64
https://www.idreamtin8bits.com/

Inspired by and code used from DraBrowse:
DraBrowse (db*) is a simple file browser.
Originally created 2009 by Sascha Bader.
Used version adapted by Dirk Jagdmann (doj)
https://github.com/doj/dracopy

Most of code and functionality ported from:
DMBoot 128:
Device Manager Boot Menu for the Commodore 128
Written in 2020-2023 by Xander Mol
https://github.com/xahmol/DMBoot

Additionally uses code from:
- Ultimate 64/II+ Command Library
  Scott Hutter, Francesco Sblendorio
  https://github.com/xlar54/ultimateii-dos-lib
- GenCart64 - cc65 Library for C64 cartridges
  Joseph Rose, a.k.a. Harry Potter
  https://sourceforge.net/projects/cc65extra/files/memory%20cfgs/
  Used for inspiration for building C64 cartridge image with CC65
- Sidekick64 by frntc
  https://github.com/frntc/Sidekick64/blob/master/Source/Firmware/C64Side/cart.a
  Used as inspiration for cartridge init and exit code
- UUC based on Final Cartridge III by Bart van Leeuwen / bvl1999
  https://github.com/bvl1999/final_cartridge/blob/master/core/init.s
  Used for inspiration for building C64 cartridge image with CC65
  Also help with ideas, feednack, error solving, testing and code snippets
- ntp2ultimate by MaxPlap
  https://github.com/MaxPlap/ntp2ultimate
  Time via NTP code
- EPOCH-to-time-date-converter by sidsingh78
  https://github.com/sidsingh78/EPOCH-to-time-date-converter/blob/master/epoch_conv.c
- petcom version 1.00 by Craig Bruce, 18-May-1995
  Convert from PETSCII to ASCII, or vice-versa.
  https://codebase64.org/doku.php?id=source_conversion

Requires and made possible by the Ultimate II+ cartridge,
Created by Gideon Zweijtzer
https://ultimate64.com/

Licensed under the GNU General Public License v3.0

The code can be used freely as long as you retain
a notice describing original source and author.

THE PROGRAMS ARE DISTRIBUTED IN THE HOPE THAT THEY WILL BE USEFUL,
BUT WITHOUT ANY WARRANTY. USE THEM AT YOUR OWN RISK!
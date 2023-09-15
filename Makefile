# UBoot64
# Boot Menu for Ultimate equiped C64 machines
# Written in 2023 by Xander Mol
# https://github.com/xahmol/DMBoot
# https://www.idreamtin8bits.com/

# See src/main.c and src/dmbconfig.c for full credits

# Prerequisites for building:
# - CC65 compiled and included in path with sudo make avail
# - ZIP packages installed: sudo apt-get install zip
# - wput command installed: sudo apt-get install wput

SOURCESMAIN = src/main.c src/core.c src/fileio.c src/slotmenu.c src/time.c src/filebrowse.c src/petscii_ascii.c src/ultimate_common_lib.c src/ultimate_dos_lib.c src/ultimate_time_lib.c src/ultimate_network_lib.c
LIBMAIN = src/bankswitch.s src/fc3.s
README = README.pdf
ZIP = UBoot64-v091-$(shell date "+%Y%m%d-%H%M").zip

# Hostname of Ultimate II+ target for deployment. Edit for proper IP and usb number
ULTHOST = ftp://192.168.1.55/Flash/carts/

MAIN = uboot64.crt
DEPLOYS = $(MAIN)

CC65_TARGET = c64
CC = cl65
CFLAGS  = -t $(CC65_TARGET) --create-dep $(<:.c=.d) -O -I include
LDFLAGSMAIN = -C fc3.cfg -t $(CC65_TARGET) -m $(MAIN).map

########################################

.SUFFIXES:
.PHONY: all clean deploy
all: $(MAIN)

ifneq ($(MAKECMDGOALS),clean)
-include $(SOURCESMAIN:.c=.d) $(SOURCESUPD:.c=.d) $(SOURCESTIME:.c=.d) $(SOURCESGEOS:.c=.d) $(SOURCESCFG:.c=.d)
endif

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<
  
$(MAIN): $(LIBMAIN) $(SOURCESMAIN:.c=.o)
	$(CC) $(LDFLAGSMAIN) -o $@ $^

#$(ZIP): $(DEPLOYS) $(README)
#	zip $@ $^

clean:
	$(RM) $(SOURCESMAIN:.c=.o) $(SOURCESMAIN:.c=.d) $(MAIN) $(MAIN).map
	
# To deploy software to UII+ enter make deploy. Obviously C128 needs to powered on with UII+ and USB drive connected.
deploy: $(MAIN) $(UPDATE)
	wput -u $(DEPLOYS) $(ULTHOST)

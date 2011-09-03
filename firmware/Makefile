### Project configuration ###

# The main project file (without extension)
PROGNAME = main


### Microcontroller and programmer configuration ###

# $(MCU) is passed to GCC
MCU = atmega8

# $(PROGRAMMER_MCU) is passed to avrdude
PROGRAMMER_MCU = atmega8

# For parallel port programmer:
#AVRDUDE_PARAMS = -p $(PROGRAMMER_MCU) -c bsd -P /dev/parport0 -E noreset
# For USBasp
AVRDUDE_PARAMS = -p $(PROGRAMMER_MCU) -c usbasp


### Compiling tools configuration ###

AS  = avr-as
CC  = avr-gcc
CXX = avr-g++

NM      = avr-nm
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
SIZE    = avr-size

VUSBDIR = vusb-20100715/usbdrv

CFLAGS   = -mmcu=$(MCU) -std=c99 -pipe -Os -Wall
CFLAGS  += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS  += -I./ -I$(VUSBDIR)
CXXFLAGS = $(CFLAGS)
ASFLAGS  = -Wa,-adhlns=$(subst $(suffix $<),.lst,$<)
LDFLAGS  = -Wl,-Map=$(PROGNAME).map
#LDFLAGS += $(GENTOO_LD_PATH_WORKAROUND)

# See:
# http://bugs.gentoo.org/show_bug.cgi?id=147155
# http://www.avrfreaks.net/index.php?name=PNphpBB2&file=viewtopic&p=336170

#GENTOO_LD_PATH_WORKAROUND = -L/usr/i686-pc-linux-gnu/avr/lib
#GENTOO_LD_PATH_WORKAROUND = -L/usr/x86_64-pc-linux-gnu/avr/lib

# ln -snf ../../lib/binutils/avr/2.*/ldscripts/ /usr/avr/lib/ldscripts


### Make targets ###

#Basic rules
.PHONY: all help clean writeflash writeeeprom writefuse erase dump comments

all: $(PROGNAME).hex $(PROGNAME).eep $(PROGNAME).lss
	./checksize $(PROGNAME).elf

help:
	@echo 'make all         - Builds the project'
	@echo 'make clean       - Deletes all built files'
	@echo
	@echo 'make writeflash  - Writes the flash memory of the microcontroller'
	@echo 'make writeeeprom - Writes the EEPROM of the microcontroller'
	@echo 'make writefuse   - Writes the fuse bytes of the microcontroller'
	@echo 'make erase       - Performs a chip erase'
	@echo 'make dump        - Dumps all memory from the microcontroller'
	@echo
	@echo 'make comments    - Prints all TODO/FIXME/XXX comments'

clean:
	rm -f $(PROGNAME).{o,s,elf,hex,eep,lss,sym,lst,map}
	rm -f $(VUSBDIR)/{oddebug,usbdrv,usbdrvasm}.{o,s,lst,map}

comments:
	ack --nomake 'TODO|FIXME|XXX'
# I could have used grep, but ack is so much easier and prettier!
# http://betterthangrep.com/
# http://search.cpan.org/dist/ack/
#
# Note: Debian (and Ubuntu?) users have this tool installed as "ack-grep"

writeflash:
	avrdude $(AVRDUDE_PARAMS) \
		-U flash:w:$(PROGNAME).hex:i

writeeeprom:
	avrdude $(AVRDUDE_PARAMS) \
		-U eeprom:w:$(PROGNAME).eep:i

writefuse:
	avrdude $(AVRDUDE_PARAMS) \
		-U hfuse:w:0xC9:m \
		-U lfuse:w:0x9F:m
# Note: lfuse should work as either 0x9F or 0xEF
# Note: avrdude supports binary (0b prefix), hexadecimal (0x prefix),
#       octal (0 prefix) and decimal (no prefix)
# Note: it might be a good idea to enable EESAVE bit (by programming it to zero)

erase:
	avrdude $(AVRDUDE_PARAMS) \
		-e
dump:
	avrdude $(AVRDUDE_PARAMS) \
		-U flash:r:flash.dump:i \
		-U eeprom:r:eeprom.dump:i \
		-U hfuse:r:hfuse.dump:b \
		-U lfuse:r:lfuse.dump:b \
		-U lock:r:lock.dump:b \
		-U signature:r:signature.dump:h


# Dependencies
$(PROGNAME).s: $(PROGNAME).c usbconfig.h
$(PROGNAME).o: $(PROGNAME).c usbconfig.h
$(PROGNAME).elf: $(PROGNAME).o $(VUSBDIR)/usbdrv.o $(VUSBDIR)/usbdrvasm.o $(VUSBDIR)/oddebug.o


# The variables:
# $@ - The name of the target of the rule.
# $? - The names of all the prerequisites that are newer than the target.
# $< - The name of the first prerequisite.
# $^ - The names of all the prerequisites.

# Pattern-rules:
%.s: %.c
	$(CC) -S $(CPPFLAGS) $(CFLAGS) $(ASFLAGS) -o $@ $<
%.o: %.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $(ASFLAGS) -o $@ $<
%.o: %.cpp
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $(ASFLAGS) -o $@ $<
%.o: %.S
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $(ASFLAGS) -o $@ $<
%.o: %.h
%.elf: %.o
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ $^

# Create final output files (.hex, .eep) from ELF output file.
# Use -O ihex for Intel Hex format
# Use -O binary for binary format
%.hex: %.elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@
	#$(OBJCOPY) -j .text -j .data -O ihex $< $@
%.eep: %.elf
	$(OBJCOPY) -j .eeprom --set-section-flags=.eeprom="alloc,load" \
	--change-section-lma .eeprom=0 -O ihex $< $@

# Create extended listing file from ELF output file.
%.lss: %.elf
	$(OBJDUMP) -h -S -C $< > $@

# Create a symbol table from ELF output file.
%.sym: %.elf
	$(NM) -n $< > $@


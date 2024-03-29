###############################################################################
# Makefile für das Viertsemester-Projekt am ACS
###############################################################################

## General Flags
TARGET = auto
MCU = atmega32
CC = avr-gcc

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -Wall -g -std=gnu99 -DF_CPU=16000000UL -Os -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d 

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)

## Linker flags
LDFLAGS = $(COMMON)
LDFLAGS +=  -Wl,-Map=$(TARGET).map

## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom -R .fuse -R .lock -R .signature
HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings

## Include Directories
INCLUDES = 

## Objects that must be built in order to link
OBJECTS = rotary.o lcd.o main.o pid.o adc.o uart.o

## Objects explicitly added by the user
LINKONLYOBJECTS = 

## Build
all: $(TARGET).elf $(TARGET).hex $(TARGET).eep $(TARGET).lss size

## Link
$(TARGET).elf: $(OBJECTS)
	 $(CC) $(LDFLAGS) $(OBJECTS) $(LINKONLYOBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET).elf

%.hex: $(TARGET).elf
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS)  $< $@

%.eep: $(TARGET).elf
	avr-objcopy $(HEX_EEPROM_FLAGS) -O ihex $< $@ || exit 0

%.lss: $(TARGET).elf
	avr-objdump -h -S $< > $@

size: $(TARGET).elf
	@echo
	@avr-size -C --mcu=${MCU} ${TARGET}.elf

flash: $(TARGET).hex
	avrdude -p m32 -c avrisp2 -P usb \
	-U flash:w:$(TARGET).hex

eeprom: $(TARGET).hex
	avrdude -p m32 -c avrisp2 -P usb \
	-U eeprom:w:$(TARGET).eep

## Clean target
.PHONY: clean
clean:
	rm -rf $(OBJECTS) dep/*
	for suf in elf hex eep lss map ; do \
		rm -f $(TARGET).$$suf ; \
	done


## Other dependencies
-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)


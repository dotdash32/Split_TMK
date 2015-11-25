# Target file name (without extension).
TARGET = split_keyboard_slave

# Directory common source filess exist
TMK_DIR = ../../../tmk_core

# Directory keyboard dependent files exist
TARGET_DIR = .

# project specific files
SRC =	\
	main.c \
	matrix.c \
	clock.c \
	wireless/crypto.c \
	wireless/nrf.c \
	wireless/aes/aes_dec-asm.S \
	wireless/aes/aes_enc-asm.S \
	wireless/aes/aes_invsbox-asm.S \
	wireless/aes/aes_keyschedule-asm.S \
	wireless/aes/aes_sbox-asm.S \

# CONFIG_H = config.h

# MCU name
MCU = atmega328p

ifndef COM_PORT
    COM_PORT=/dev/ttyACM0
endif
AVRDUDE_CMD=avrdude -p $(MCU) -c avrisp -b 19200 -P $(COM_PORT)
# PROGRAM_CMD=$(AVRDUDE_CMD)

# Oscillator frequency.
F_OSC = 8000000
# Processor frequency.
F_CPU = 2000000

# Optimisation level
OPT = 3

OPT_DEFS += -DUSE_AUTO_ACK=1
OPT_DEFS += -DF_OSC=$(F_OSC)UL

EEPROM_FILE = ../eeprom-split.eep

# Search Path
VPATH += $(TARGET_DIR)
VPATH += $(TARGET_DIR)/..

include $(TMK_DIR)/rules.mk

dependencies:
	# Create object files directory
	$(shell mkdir $(OBJDIR) 2>/dev/null)
	# Include the dependency files.
	-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)

build-left: clean
build-left: dependencies
build-left: OPT_DEFS += -DCE=1
build-left: OPT_DEFS += -DCSN=2
build-left: OPT_DEFS += -DDEVICE_ID=0
build-left: hex

build-right: clean
build-right: dependencies
build-right: OPT_DEFS += -DCE=1
build-right: OPT_DEFS += -DCSN=2
build-right: OPT_DEFS += -DDEVICE_ID=1
build-right: hex

program-left: build-left
program-left: program-slave
program-left: lock

program-right: build-right
program-right: program-slave
program-right: lock

uno:
	avrdude -c arduino -p atmega328p -P $(COM_PORT) \
		-U flash:w:build-uno/wireless-slave.hex:i

fuses:
	$(AVRDUDE_CMD) \
		-U lfuse:w:0x42:m \
		-U hfuse:w:0xd9:m \
		-U efuse:w:0x06:m

lock:
	# disable reading and writing to flash and eeprom
	$(AVRDUDE_CMD) \
		-U lock:w:0x3c:m

program-slave:
	$(AVRDUDE_CMD) -U flash:w:$(TARGET).hex:i
	sleep 1
	$(AVRDUDE_CMD) -U eeprom:w:$(EEPROM_FILE):i


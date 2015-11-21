NO_CORE = yes
MCU = atmega328p

# AVRDUDE_OPTS += -c arduino

# AVRDUDE_ARD_BAUDRATE = 57600
# AVRDUDE_ARD_PROGRAMMER = arduino

# ISP_PROG   = avrisp
# AVRDUDE_ISP_BAUDRATE = 19200

# F_OSC = 16000000L # either 8 or 16 MHz
F_OSC = 8000000L # either 8 or 16 MHz
F_CPU = 2000000L # speed we run at when doing work

CFLAGS += \
	  -std=gnu99 \

OPTIMIZATION_LEVEL = 3

CPPFLAGS += -DMONITOR_BAUDRATE=$(MONITOR_BAUDRATE) \
	    -DUSE_AUTO_ACK=1 \
	    -DDEVICE_ID=$(DEVICE_ID) \
	    -DF_OSC=$(F_OSC)

EEPROM_FILE = ../eeprom-split.eep
COM_PORT = /dev/ttyACM0
PROGRAM_CMD = avrdude -c $(ISP_PROG) -p atmega328p -P $(COM_PORT) -b $(AVRDUDE_ISP_BAUDRATE)


### or on Linux: (remove the one you don't want)
ARDUINO_DIR       = /usr/share/arduino

include $(ARDUINO_DIR)/Arduino.mk


build-left: CPPFLAGS += -DCE=1
build-left: CPPFLAGS += -DCSN=2
build-left: clean
build-left: DEVICE_ID=0
build-left: all

build-right: CPPFLAGS += -DCE=1
build-right: CPPFLAGS += -DCSN=2
build-right: clean
build-right: DEVICE_ID=1
build-right: all

program-left: build-left
program-left: program

program-right: build-right
program-right: program

uno:
	avrdude -c arduino -p atmega328p -P $(COM_PORT) \
		-U flash:w:build-uno/wireless-slave.hex:i

set-fuses:
	$(PROGRAM_CMD) \
		-U lfuse:w:0x42:m \
		-U hfuse:w:0xd9:m \
		-U efuse:w:0x06:m

# program-lock:
# 	avrdude -c $(ISP_PROG) -p atmega328p -P $(COM_PORT) \
# 		-b $(AVRDUDE_ISP_BAUDRATE) \
# 		-U flash:w:build-uno/wireless-slave.hex:i

# program-left: program DEVICE_NUMBER=0
# program-right: program DEVICE_NUMBER=1

program: all
	$(PROGRAM_CMD) -U flash:w:build-uno/wireless-slave.hex:i
	sleep 1
	$(PROGRAM_CMD) -U eeprom:w:$(EEPROM_FILE):i

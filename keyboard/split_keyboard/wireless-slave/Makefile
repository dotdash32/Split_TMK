NO_CORE = yes
MCU = atmega328p

# AVRDUDE_OPTS += -c arduino

# AVRDUDE_ARD_BAUDRATE = 19200
# AVRDUDE_ARD_PROGRAMMER = arduino

ISP_PROG   = avrisp
AVRDUDE_ISP_BAUDRATE = 19200

F_OSC = 16000000L # either 8 or 16 MHz
F_CPU = 2000000L # speed we run at when doing work

CFLAGS += \
	  -std=gnu99 \

OPTIMIZATION_LEVEL = 3

CPPFLAGS += -DMONITOR_BAUDRATE=$(MONITOR_BAUDRATE) \
	    -DUSE_AUTO_ACK=1 \
	    -DMASTER_DEVICE=0 \
	    -DDEVICE_NUMBER=$(DEVICE_NUMBER) \
	    -DF_OSC=$(F_OSC)

### or on Linux: (remove the one you don't want)
ARDUINO_DIR       = /usr/share/arduino

include $(ARDUINO_DIR)/Arduino.mk

program:
	avrdude -c arduino -p atmega328p -P /dev/ttyACM0 \
		-U flash:w:build-uno/wireless-slave.hex:i

program-left: all
	avrdude -c $(ISP_PROG) -p atmega328p -P /dev/ttyACM0 \
		-b $(AVRDUDE_ISP_BAUDRATE) -U eeprom:w:../eeprom-left.eep:i \
		-U flash:w:build-uno/wireless-slave.hex:i

program-right: all
	avrdude -c $(ISP_PROG) -p atmega328p -P /dev/ttyACM0 \
		-b $(AVRDUDE_ISP_BAUDRATE) -U eeprom:w:../eeprom-right.eep:i \
		-U flash:w:build-uno/wireless-slave.hex:i

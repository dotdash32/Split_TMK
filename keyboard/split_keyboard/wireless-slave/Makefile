BOARD_TAG    = uno
F_CPU = 500000L

ISP_PROG   = avrisp
AVRDUDE_ISP_BAUDRATE = 19200

CFLAGS += \
	  -std=gnu99 \


CPPFLAGS += -DMONITOR_BAUDRATE=$(MONITOR_BAUDRATE) \
	    -DUSE_AUTO_ACK=1 \
	    -DMASTER_DEVICE=0 \
	    -DDEBUG \
	    -DVERBOSE \


OPTIMIZATION_LEVEL = 3

### or on Linux: (remove the one you don't want)
ARDUINO_DIR       = /usr/share/arduino

include $(ARDUINO_DIR)/Arduino.mk

.PHONY:
program:
	sleep 3;
	avrdude -p atmega32u4 -P /dev/ttyACM0 -c avr109 -U flash:w:build-$(BOARD_TAG)/$(TARGET).hex

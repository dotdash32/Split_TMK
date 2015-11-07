BOARD_TAG = uno

# ISP_PROG   = avrisp
# AVRDUDE_ISP_BAUDRATE = 19200

F_OSC = 16000000L # either 8 or 16 MHz
F_CPU = 1000000L

CFLAGS += \
	  -std=gnu99 \

# left    = 0
# right 1 = 1
DEVICE_NUMBER = 0

OPTIMIZATION_LEVEL = 3

CPPFLAGS += -DMONITOR_BAUDRATE=$(MONITOR_BAUDRATE) \
	    -DUSE_AUTO_ACK=1 \
	    -DMASTER_DEVICE=0 \
	    -DDEVICE_NUMBER=$(DEVICE_NUMBER) \
	    -DF_OSC=$(F_OSC)

### or on Linux: (remove the one you don't want)
ARDUINO_DIR       = /usr/share/arduino

include $(ARDUINO_DIR)/Arduino.mk

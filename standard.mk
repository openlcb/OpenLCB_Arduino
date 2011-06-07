# Simple makefile fragment for Arduino sketch, somewhat brute force.
# Requires a suitable arduino.mk has been included first

INCLUDE_ARDUINO := ${INCLUDE_ARDUINO} -I${ARDUINO_ROOT}libraries/EEPROM
INCLUDE_ARDUINO := ${INCLUDE_ARDUINO} -I${ARDUINO_ROOT}libraries/LiquidCrystal
INCLUDE_ARDUINO := ${INCLUDE_ARDUINO} -I${ARDUINO_ROOT}libraries/SPI
INCLUDE_ARDUINO := ${INCLUDE_ARDUINO} -I${ARDUINO_ROOT}libraries/Servo

# note that this in meant to be included one level down, where PWD refers to a specific examples/sketch

# local libraries
INCLUDE_OPTIONS := -I${PWD}/libraries/OlcbArduinoCAN 
INCLUDE_OPTIONS := ${INCLUDE_OPTIONS} -I${PWD}/libraries/OlcbCommonCAN 
INCLUDE_OPTIONS := ${INCLUDE_OPTIONS} -I${PWD}/libraries/CAN 
INCLUDE_OPTIONS := ${INCLUDE_OPTIONS} -I${PWD}/libraries/LCmini
INCLUDE_OPTIONS := ${INCLUDE_OPTIONS} -I${PWD}/libraries/LocoNet
INCLUDE_OPTIONS := ${INCLUDE_OPTIONS} -I${PWD}/libraries/NmraDcc
INCLUDE_OPTIONS := ${INCLUDE_OPTIONS} -I${PWD}/libraries/EthernetBonjour
INCLUDE_OPTIONS := ${INCLUDE_OPTIONS} -I${PWD}/libraries/EthernetDNS
INCLUDE_OPTIONS := ${INCLUDE_OPTIONS} -I${PWD}/libraries/EthernetDHCP

# libraries that locals may sub for

INCLUDE_OPTIONS := ${INCLUDE_OPTIONS} -I${PWD}/libraries/EtherNet2 
INCLUDE_OPTIONS := ${INCLUDE_OPTIONS} -I${ARDUINO_ROOT}libraries/Ethernet

TMPDIR := $(shell mktemp -d /tmp/Arduino.XXXXXXXX)

# assumes only one PDE file
PDE := $(foreach V,$(wildcard *.pde),$(V:.pde=))

# copy header files, if any, to temp build directory
$(foreach V,$(wildcard *.h),${TMPDIR}/$(V)):
	cp *.h ${TMPDIR}/

all: $(foreach V,$(wildcard *.h),${TMPDIR}/$(V))
	cp ${PDE}.pde ${TMPDIR}/${PDE}.cpp
	cd ${TMPDIR}; ${ARDUINO_ROOT}hardware/tools/avr/bin/avr-g++ -c -g  ${CC_OPTIONS_ARDUINO} ${INCLUDE_ARDUINO} ${INCLUDE_OPTIONS} *.cpp -o /dev/null
	rm -r ${TMPDIR}
# Simple makefile fragment for Arduino sketch, somewhat brute force.

# Arduino install definitions, for Arduino 18
# The ARDUINO_ROOT definition varies with install location
ARDUINO_ROOT := /Applications/Arduino.app/Contents/Resources/Java/
CC_OPTIONS_ARDUINO := -Os -w -fno-exceptions -ffunction-sections -fdata-sections -std=gnu99 -mmcu=atmega168 -DF_CPU=16000000L

INCLUDE_ARDUINO := -I${ARDUINO_ROOT}hardware/arduino/cores/arduino 

INCLUDE_ARDUINO := ${INCLUDE_ARDUINO} -I${ARDUINO_ROOT}libraries/EEPROM
INCLUDE_ARDUINO := ${INCLUDE_ARDUINO} -I${ARDUINO_ROOT}libraries/LiquidCrystal
INCLUDE_ARDUINO := ${INCLUDE_ARDUINO} -I${ARDUINO_ROOT}libraries/Servo

# note that this in meant to be included one level down, where PWD refers to a specific examples/sketch

# local libraries
INCLUDE_OPTIONS := -I${PWD}/libraries/OlcbArduinoCAN 
INCLUDE_OPTIONS := ${INCLUDE_OPTIONS} -I${PWD}/libraries/OlcbCommonCAN 
INCLUDE_OPTIONS := ${INCLUDE_OPTIONS} -I${PWD}/libraries/CAN 
INCLUDE_OPTIONS := ${INCLUDE_OPTIONS} -I${PWD}/libraries/EtherNet2 
INCLUDE_OPTIONS := ${INCLUDE_OPTIONS} -I${PWD}/libraries/LCmini
INCLUDE_OPTIONS := ${INCLUDE_OPTIONS} -I${PWD}/libraries/LocoNet
INCLUDE_OPTIONS := ${INCLUDE_OPTIONS} -I${PWD}/libraries/NmraDcc

TMPDIR := $(shell mktemp -d /tmp/Arduino.XXXXXXXX)

PDE := $(foreach V,$(wildcard *.pde),$(V:.pde=))

all:
	cp ${PDE}.pde ${TMPDIR}/${PDE}.cpp
	-cp *.h ${TMPDIR}/ >& /dev/null
	cd ${TMPDIR}; ${ARDUINO_ROOT}hardware/tools/avr/bin/avr-g++ -c -g  ${CC_OPTIONS_ARDUINO} ${INCLUDE_ARDUINO} ${INCLUDE_OPTIONS} *.cpp -o /dev/null
	rm -r ${TMPDIR}
# Simple makefile fragment for Arduino sketch, somewhat brute force.
# Requires a suitable arduino.mk has been included first

INCLUDE_ARDUINO := ${INCLUDE_ARDUINO} -I${ARDUINO_ROOT}libraries/EEPROM
INCLUDE_ARDUINO := ${INCLUDE_ARDUINO} -I${ARDUINO_ROOT}libraries/LiquidCrystal
INCLUDE_ARDUINO := ${INCLUDE_ARDUINO} -I${ARDUINO_ROOT}libraries/SPI
INCLUDE_ARDUINO := ${INCLUDE_ARDUINO} -I${ARDUINO_ROOT}libraries/Servo

# note that this in meant to be included one level down, where PWD refers to a specific examples/sketch

# local libraries
INCLUDE_OPTIONS :=                    -I${PWD}/libraries/ButtonLED 
INCLUDE_OPTIONS := ${INCLUDE_OPTIONS} -I${PWD}/libraries/OlcbArduinoCAN 
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

io: TARGET = at90can128
io: MCU_TARGET = at90can128
io: CFLAGS += '-DMAX_TIME_COUNT=F_CPU>>4' '-DNUM_LED_FLASHES=3'
#  -DBAUD_RATE=57600
io: AVR_FREQ = 16000000L
#io: LDSECTION  = --section-start=.text=0xF000
io: LDSECTION  = --section-start=.text=0x1E000
io: ${INO}${PDE}_io.hex

%.hex: %.elf
	$(OBJCOPY) -j .text -j .data -O ihex $< $@

# assumes only one PDE or INO file
PDE := $(foreach V,$(wildcard *.pde),$(V:.pde=))
INO := $(foreach V,$(wildcard *.ino),$(V:.ino=))

# copy header files, if any, to temp build directory
$(foreach V,$(wildcard *.h),${TMPDIR}/$(V)):
	cp *.h ${TMPDIR}/

all: $(foreach V,$(wildcard *.h),${TMPDIR}/$(V))
	-cp ${INO}.ino ${TMPDIR}/${INO}.cpp
	-cp ${PDE}.pde ${TMPDIR}/${PDE}.cpp
	cd ${TMPDIR}; ${ARDUINO_ROOT}hardware/tools/avr/bin/avr-g++ -c -g  ${CC_OPTIONS_ARDUINO} ${INCLUDE_ARDUINO} ${INCLUDE_OPTIONS} *.cpp 

#	rm -r ${TMPDIR}




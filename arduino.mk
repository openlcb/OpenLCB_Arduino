# Arduino install definitions, for Arduino 22

# The ARDUINO_ROOT definition varies with install location
ARDUINO_ROOT := /Applications/Arduino.app/Contents/Resources/Java/

CC_OPTIONS_ARDUINO := -Os -w -fno-exceptions -ffunction-sections -fdata-sections -std=gnu99 -mmcu=atmega168 -DF_CPU=16000000L

INCLUDE_ARDUINO := -I${ARDUINO_ROOT}hardware/arduino/cores/arduino 

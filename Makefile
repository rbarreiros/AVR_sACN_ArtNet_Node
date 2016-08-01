
ARDUINO_DIR   = /home/rbarreiros/Projectos/Electronica/Arduino
USER_LIB_PATH = ./lib
BOARDS_TXT = ./board/boards.txt
BOARD_TAG     = atmega328_20
ISP_PROG      = usbasp
AVRDUDE_ISP_OPTS = -F

ARDUINO_LIBS = EEPROMEx ethercard ArduinoArtNet DMXSerial

include /home/rbarreiros/Projectos/Electronica/Arduino-Makefile/Arduino.mk

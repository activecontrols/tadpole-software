#ifndef TEENSY_PINS_H
#define TEENSY_PINS_H

// https://www.pjrc.com/teensy/card11a_rev3_web.png

// TODO RJN teensy - set pins

#define ZUCROW_PANIC_PIN 14
#define ZUCROW_SYNC_PIN 15
#define TEENSY_PANIC_PIN 16
#define TEENSY_SYNC_PIN 17

// Demux System

#define SPI_DEMUX_BIT_0 37 // marked as CS_BIN_3
#define SPI_DEMUX_BIT_1 36 // marked as CS_BIN_2
#define SPI_DEMUX_BIT_2 35 // marked as CS_BIN_1
#define SPI_DEMUX_BIT_3 34 // marked as CS_BIN_0
#define SPI_DEMUX_BIT_4 33 // marked as CS_BIN_4

#endif
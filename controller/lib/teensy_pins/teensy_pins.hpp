#ifndef TEENSY_PINS_H
#define TEENSY_PINS_H

// https://www.pjrc.com/teensy/card11a_rev3_web.png

// TODO RJN teensy - set pins

#define ZUCROW_PANIC_PIN 14
#define ZUCROW_SYNC_PIN 15
#define TEENSY_PANIC_PIN 16
#define TEENSY_SYNC_PIN 17

// Demux System

#define SPI_DEMUX_BIT_0 29 // marked as CS_BIN_4
#define SPI_DEMUX_BIT_1 28 // marked as CS_BIN_0
#define SPI_DEMUX_BIT_2 4  // marked as CS_BIN_1
#define SPI_DEMUX_BIT_3 3  // marked as CS_BIN_2
#define SPI_DEMUX_BIT_4 2  // marked as CS_BIN_3

// Other
// SPI MISO 1
// SPI MOSI 26
// SPI SCK 27

#endif
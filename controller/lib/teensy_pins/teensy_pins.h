#ifndef TEENSY_PINS_H
#define TEENSY_PINS_H

// https://www.pjrc.com/teensy/card11a_rev3_web.png

// from zucrow (to teensy)
#define ZI_FROM_ZUCROW_EXTRA_1 14
#define ZUCROW_SYNC_PIN 15
#define ZUCROW_PANIC_PIN 16
#define ZI_FROM_ZUCROW_EXTRA_2 17

// from teensy (to zucrow)
#define ZI_TO_TEENSY_EXTRA_1 18
#define TEENSY_SYNC_PIN 19
#define TEENSY_PANIC_PIN 20
#define ZI_TO_TEENSY_EXTRA_2 21

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
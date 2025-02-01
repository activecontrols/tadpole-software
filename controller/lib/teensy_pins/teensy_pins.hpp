#ifndef TEENSY_PINS_H
#define TEENSY_PINS_H

// https://www.pjrc.com/teensy/card11a_rev3_web.png

// These will get replaced by ADC stuff
#define LOX_PRESSURE_UPSTREAM_PIN 21
#define LOX_PRESSURE_DOWNSTREAM_PIN 20

#define ZUCROW_PANIC_PIN 14
#define ZUCROW_SYNC_PIN 15
#define TEENSY_PANIC_PIN 16
#define TEENSY_SYNC_PIN 17

// This will be replaced by the demux system
#define DAC_CS_PIN 18

// Demux System
#define SPI_DEMUX_BIT_0 41
#define SPI_DEMUX_BIT_1 40
#define SPI_DEMUX_BIT_2 39
#define SPI_DEMUX_BIT_3 38

#endif
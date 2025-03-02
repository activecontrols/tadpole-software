#ifndef SPI_DEMUX_H
#define SPI_DEMUX_H

#include <Arduino.h>

#define SPI_DEVICE_TC_0 0
#define SPI_DEVICE_TC_1 1
#define SPI_DEVICE_PT_0 4
#define SPI_DEVICE_PT_1 5

#define SPI_DEVICE_ZUCROW_DAC 24
//      SPI_DEVICE_ZUCROW_DAC 25 // either works
//      SPI_DEVICE_NULL 26
//      SPI_DEVICE_NULL 27
//      SPI_DEVICE_NULL 28
//      SPI_DEVICE_NULL 29
//      SPI_DEVICE_NULL 30
#define SPI_DEVICE_NULL 31

namespace SPI_Demux {

void begin();
void select_chip(int chip_id);
void deselect_chip();
void select_chip_cmd();
void deselect_chip_cmd();

} // namespace SPI_Demux

#endif
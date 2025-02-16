#ifndef SPI_DEMUX_H
#define SPI_DEMUX_H

#include <Arduino.h>

#define SPI_DEVICE_PT_0 0
#define SPI_DEVICE_PT_1 1
#define SPI_DEVICE_PT_2 2
#define SPI_DEVICE_PT_3 3
#define SPI_DEVICE_TC_0 4
#define SPI_DEVICE_TC_1 5
#define SPI_DEVICE_TC_2 6
#define SPI_DEVICE_TC_3 7

#define SPI_DEVICE_ZUCROW_DAC 14
#define SPI_DEVICE_NULL 15

namespace SPI_Demux {

void begin();
void select_chip(int chip_id);
void deselect_chip();

} // namespace SPI_Demux

#endif
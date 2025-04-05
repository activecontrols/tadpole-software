#ifndef SPI_DEMUX_H
#define SPI_DEMUX_H

// only even numbers on PT boards
#define SPI_DEVICE_PT_WATER_TANK 4
#define SPI_DEVICE_PT_WATER_VENTURI_UPSTREAM 8
#define SPI_DEVICE_PT_WATER_VENTURI_THROAT 12

#define SPI_DEVICE_TC_WATER 22

// 15 - 23 unusued

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
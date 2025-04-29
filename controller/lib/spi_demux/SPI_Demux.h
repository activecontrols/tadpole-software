#ifndef SPI_DEMUX_H
#define SPI_DEMUX_H

// only even numbers on PT boards
#define SPI_DEVICE_PT_LOX_VALVE_UPSTREAM 2
// skip id 6
#define SPI_DEVICE_PT_LOX_VALVE_DOWNSTREAM 10
#define SPI_DEVICE_PT_LOX_VENTURI_DIFFERENTIAL 14

#define SPI_DEVICE_PT_IPA_VALVE_UPSTREAM 0
#define SPI_DEVICE_PT_IPA_VALVE_DOWNSTREAM 4
#define SPI_DEVICE_PT_IPA_VENTURI_DIFFERENTIAL 8

#define SPI_DEVICE_PT_CHAMBER 18

#define SPI_DEVICE_TC_LOX_VENTURI 17
#define SPI_DEVICE_TC_LOX_VALVE 17

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
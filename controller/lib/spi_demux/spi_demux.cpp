#include "spi_demux.hpp"
#include "teensy_pins.hpp"
#include "SPI.h"

void SPI_Demux::begin() {
  SPI.begin();

  pinMode(SPI_DEMUX_BIT_0, OUTPUT);
  pinMode(SPI_DEMUX_BIT_1, OUTPUT);
  pinMode(SPI_DEMUX_BIT_2, OUTPUT);
  pinMode(SPI_DEMUX_BIT_3, OUTPUT);
}

void SPI_Demux::select_chip(int chip_id) {
  digitalWrite(SPI_DEMUX_BIT_0, (bool)(chip_id & 0b00001));
  digitalWrite(SPI_DEMUX_BIT_1, (bool)(chip_id & 0b00010));
  digitalWrite(SPI_DEMUX_BIT_2, (bool)(chip_id & 0b00100));
  digitalWrite(SPI_DEMUX_BIT_3, (bool)(chip_id & 0b01000));
  digitalWrite(SPI_DEMUX_BIT_4, (bool)(chip_id & 0b10000));
}

void SPI_Demux::deselect_chip() {
  select_chip(SPI_DEVICE_NULL);
}
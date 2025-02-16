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
  digitalWrite(SPI_DEMUX_BIT_0, chip_id & 0b0001);
  digitalWrite(SPI_DEMUX_BIT_1, chip_id & 0b0010);
  digitalWrite(SPI_DEMUX_BIT_2, chip_id & 0b0100);
  digitalWrite(SPI_DEMUX_BIT_3, chip_id & 0b1000);
}

void SPI_Demux::deselect_chip() {
  select_chip(SPI_DEVICE_NULL);
}
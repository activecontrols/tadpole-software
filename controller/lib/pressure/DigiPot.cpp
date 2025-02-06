#include "DigiPot.h"
#include "spi_demux.hpp"
#include "Router.h"
#include "SPI.h"

DigiPot::DigiPot(int demuxAddr) {
  this->demuxAddr = demuxAddr;
}

uint16_t DigiPot::send_cmd(uint16_t addr, uint16_t cmd, uint16_t data) {
  uint16_t packet = 0;
  packet |= addr;
  packet |= cmd;
  if (data > 0x03FF) {
    Router::info_no_newline("Clipping digi pot data packet");
    data = 0x03FF;
  }
  packet |= data; // set all of the corresponding bits

  SPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0)); // TODO RJN DigiPot - check SPI mode
  SPI_Demux::select_chip(demuxAddr);
  delayMicroseconds(1);

  uint16_t retval = SPI.transfer16(packet);

  delayMicroseconds(1);
  SPI_Demux::deselect_chip();
  SPI.endTransaction();

  return retval;
}
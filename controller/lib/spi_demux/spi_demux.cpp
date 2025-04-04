#include "SPI_Demux.h"

#include "teensy_pins.h"
#include "SPI_Fixed.h"
#include "Router.h"

void SPI_Demux::begin() {
  SPI1.begin();

  pinMode(SPI_DEMUX_BIT_0, OUTPUT);
  pinMode(SPI_DEMUX_BIT_1, OUTPUT);
  pinMode(SPI_DEMUX_BIT_2, OUTPUT);
  pinMode(SPI_DEMUX_BIT_3, OUTPUT);
  pinMode(SPI_DEMUX_BIT_4, OUTPUT);

  Router::add({select_chip_cmd, "spi_select"});
  Router::add({deselect_chip_cmd, "spi_deselect"});
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

void SPI_Demux::select_chip_cmd() {
  Router::info_no_newline("Enter #: ");
  String respStr = Router::read(10);

  int cid;
  int result = std::sscanf(respStr.c_str(), "%d", &cid);
  if (result != 1) {
    Router::info("Could not convert input to a int, not continuing");
    return;
  }

  SPI_Demux::select_chip(cid);
  Router::info("chip selected");
}

void SPI_Demux::deselect_chip_cmd() {
  SPI_Demux::deselect_chip();
}

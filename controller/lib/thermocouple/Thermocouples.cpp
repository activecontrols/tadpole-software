#include "Thermocouples.h"
#include "spi_demux.hpp"

Thermocouple::Thermocouple(int demuxAddr) {
}

namespace TC {
Thermocouple example_tc(SPI_DEVICE_TC_0);
} // namespace TC
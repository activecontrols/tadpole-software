#include "Arduino.h"
#include "ADS131M0x.h"
#include "SPI_Fixed.h"
#include "spi_demux.hpp"

#ifdef IS_M02
#define DO_PRAGMA(x) _Pragma(#x)
#define INFO(x) DO_PRAGMA(message("\nREMARK: " #x))
// INFO Version for ADS131M02
#endif

int32_t ADS131M0x::val32Ch0 = 0x7FFFFF;

/**
 * @brief Construct a new ADS131M0x::ADS131M0x object
 *
 */
ADS131M0x::ADS131M0x(int demuxAddr) {
  this->demuxAddr = demuxAddr;
}

/**
 * @brief Write to ADC131M01 or ADC131M04 register
 *
 * @param address
 * @param value
 * @return uint8_t
 */
uint8_t ADS131M0x::writeRegister(uint8_t address, uint16_t value) {
  uint16_t res;
  uint8_t addressRcv;
  uint8_t bytesRcv;
  uint16_t cmd = 0;

  SPI1.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE1));
  SPI_Demux::select_chip(demuxAddr);
  delayMicroseconds(2);

  cmd = (CMD_WRITE_REG) | (address << 7) | 0;

  SPI1.transfer16(cmd);
  SPI1.transfer(0x00);

  SPI1.transfer16(value);
  SPI1.transfer(0x00);

  SPI1.transfer16(0x0000);
  SPI1.transfer(0x00);

  SPI1.transfer16(0x0000);
  SPI1.transfer(0x00);

#ifndef IS_M02
  SPI1.transfer16(0x0000);
  SPI1.transfer(0x00);

  SPI1.transfer16(0x0000);
  SPI1.transfer(0x00);
#endif

  res = SPI1.transfer16(0x0000);
  SPI1.transfer(0x00);

  SPI1.transfer16(0x0000);
  SPI1.transfer(0x00);

  SPI1.transfer16(0x0000);
  SPI1.transfer(0x00);

  SPI1.transfer16(0x0000);
  SPI1.transfer(0x00);
#ifndef IS_M02
  SPI1.transfer16(0x0000);
  SPI1.transfer(0x00);

  SPI1.transfer16(0x0000);
  SPI1.transfer(0x00);
#endif

  delayMicroseconds(2);
  SPI_Demux::deselect_chip();
  SPI1.endTransaction();

  addressRcv = (res & REGMASK_CMD_READ_REG_ADDRESS) >> 7;
  bytesRcv = (res & REGMASK_CMD_READ_REG_BYTES);

  if (addressRcv == address) {
    return bytesRcv + 1;
  }
  return 0;
}

/**
 * @brief
 *
 * @param address
 * @return uint16_t
 */
uint16_t ADS131M0x::readRegister(uint8_t address) {
  uint16_t cmd;
  uint16_t data;

  cmd = CMD_READ_REG | (address << 7 | 0);

  SPI1.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE1));
  SPI_Demux::select_chip(demuxAddr);
  delayMicroseconds(2);

  SPI1.transfer16(cmd);
  SPI1.transfer(0x00);

  SPI1.transfer16(0x0000);
  SPI1.transfer(0x00);

  SPI1.transfer16(0x0000);
  SPI1.transfer(0x00);

  SPI1.transfer16(0x0000);
  SPI1.transfer(0x00);
#ifndef IS_M02
  SPI1.transfer16(0x0000);
  SPI1.transfer(0x00);

  SPI1.transfer16(0x0000);
  SPI1.transfer(0x00);
#endif
  data = SPI1.transfer16(0x0000);
  SPI1.transfer(0x00);

  SPI1.transfer16(0x0000);
  SPI1.transfer(0x00);

  SPI1.transfer16(0x0000);
  SPI1.transfer(0x00);

  SPI1.transfer16(0x0000);
  SPI1.transfer(0x00);
#ifndef IS_M02
  SPI1.transfer16(0x0000);
  SPI1.transfer(0x00);

  SPI1.transfer16(0x0000);
  SPI1.transfer(0x00);
#endif

  delayMicroseconds(2);
  SPI_Demux::deselect_chip();
  SPI1.endTransaction();

  return data;
}

/**
 * @brief Write a value to the register, applying the mask to touch only the necessary bits.
 * It does not carry out the shift of bits (shift), it is necessary to pass the shifted value to the correct position
 *
 * @param address
 * @param value
 * @param mask
 */
void ADS131M0x::writeRegisterMasked(uint8_t address, uint16_t value, uint16_t mask) {
  // Read the current content of the register
  uint16_t register_contents = readRegister(address);
  // Change the mask bit by bit (it remains 1 in the bits that must not be touched and 0 in the bits to be modified)
  // An AND is performed with the current content of the record. "0" remain in the part to be modified
  register_contents = register_contents & ~mask;
  // OR is made with the value to load in the registry. value must be in the correct position (shitf)
  register_contents = register_contents | value;
  writeRegister(address, register_contents);
}

/// @brief Hardware reset (reset low activ)
/// @param reset_pin
void ADS131M0x::reset(uint8_t reset_pin) {
  pinMode(reset_pin, OUTPUT);
  digitalWrite(reset_pin, HIGH);
  delay(100);
  digitalWrite(reset_pin, LOW);
  delay(100);
  digitalWrite(reset_pin, HIGH);
  delay(1);
}

/**
 * @brief read status cmd-register
 *
 * @return uint16_t
 */
uint16_t ADS131M0x::isResetOK(void) {
  return (readRegister(CMD_RESET));
}

/**
 * @brief software test of ADC data is ready
 *
 * @param channel
 * @return int8_t
 */
int8_t ADS131M0x::isDataReadySoft(byte channel) {
  if (channel == 0) {
    return (readRegister(REG_STATUS) & REGMASK_STATUS_DRDY0);
  } else if (channel == 1) {
    return (readRegister(REG_STATUS) & REGMASK_STATUS_DRDY1);
  }
#ifndef IS_M02
  else if (channel == 2) {
    return (readRegister(REG_STATUS) & REGMASK_STATUS_DRDY2);
  } else if (channel == 3) {
    return (readRegister(REG_STATUS) & REGMASK_STATUS_DRDY3);
  }
#endif
  return -1;
}

/**
 * @brief read reset status (see datasheet)
 *
 * @return true
 * @return false
 */
bool ADS131M0x::isResetStatus(void) {
  return (readRegister(REG_STATUS) & REGMASK_STATUS_RESET);
}

/**
 * @brief read locked status (see datasheet)
 *
 * @return true
 * @return false
 */
bool ADS131M0x::isLockSPI(void) {
  return (readRegister(REG_STATUS) & REGMASK_STATUS_LOCK);
}

/**
 * @brief set DRDY format (see datasheet)
 *
 * @param drdyFormat
 * @return true
 * @return false
 */
bool ADS131M0x::setDrdyFormat(uint8_t drdyFormat) {
  if (drdyFormat > 1) {
    return false;
  } else {
    writeRegisterMasked(REG_MODE, drdyFormat, REGMASK_MODE_DRDY_FMT);
    return true;
  }
}

/**
 * @brief set DRDY state (see datasheet)
 *
 * @param drdyState
 * @return true
 * @return false
 */
bool ADS131M0x::setDrdyStateWhenUnavailable(uint8_t drdyState) {
  if (drdyState > 1) {
    return false;
  } else {
    writeRegisterMasked(REG_MODE, drdyState < 1, REGMASK_MODE_DRDY_HiZ);
    return true;
  }
}

/**
 * @brief set power mode (see datasheet)
 *
 * @param powerMode
 * @return true
 * @return false
 */
bool ADS131M0x::setPowerMode(uint8_t powerMode) {
  if (powerMode > 3) {
    return false;
  } else {
    writeRegisterMasked(REG_CLOCK, powerMode, REGMASK_CLOCK_PWR);
    return true;
  }
}

/**
 * @brief set OSR digital filter (see datasheet)
 *
 * @param osr
 * @return true
 * @return false
 */
bool ADS131M0x::setOsr(uint16_t osr) {
  if (osr > 7) {
    return false;
  } else {
    writeRegisterMasked(REG_CLOCK, osr << 2, REGMASK_CLOCK_OSR);
    return true;
  }
}

/**
 * @brief input channel enable
 *
 * @param channel
 * @param enable
 * @return true
 * @return false
 */
bool ADS131M0x::setChannelEnable(uint8_t channel, uint16_t enable) {
  if (channel > 3) {
    return false;
  }
  if (channel == 0) {
    writeRegisterMasked(REG_CLOCK, enable << 8, REGMASK_CLOCK_CH0_EN);
    return true;
  } else if (channel == 1) {
    writeRegisterMasked(REG_CLOCK, enable << 9, REGMASK_CLOCK_CH1_EN);
    return true;
  }
#ifndef IS_M02
  else if (channel == 2) {
    writeRegisterMasked(REG_CLOCK, enable << 10, REGMASK_CLOCK_CH2_EN);
    return true;
  } else if (channel == 3) {
    writeRegisterMasked(REG_CLOCK, enable << 11, REGMASK_CLOCK_CH3_EN);
    return true;
  }
#endif
  return false;
}

/**
 * @brief set gain per channel (see datasheet)
 *
 * @param channel
 * @param pga
 * @return true
 * @return false
 */
bool ADS131M0x::setChannelPGA(uint8_t channel, uint16_t pga) {
  if (channel == 0) {
    writeRegisterMasked(REG_GAIN, pga, REGMASK_GAIN_PGAGAIN0);
    return true;
  } else if (channel == 1) {
    writeRegisterMasked(REG_GAIN, pga << 4, REGMASK_GAIN_PGAGAIN1);
    return true;
  }
#ifndef IS_M02
  else if (channel == 2) {
    writeRegisterMasked(REG_GAIN, pga << 8, REGMASK_GAIN_PGAGAIN2);
    return true;
  } else if (channel == 3) {
    writeRegisterMasked(REG_GAIN, pga << 12, REGMASK_GAIN_PGAGAIN3);
    return true;
  }
#endif
  return false;
}

/// @brief Set global Chop (see datasheet)
/// @param global_chop
void ADS131M0x::setGlobalChop(uint16_t global_chop) {
  writeRegisterMasked(REG_CFG, global_chop << 8, REGMASK_CFG_GC_EN);
}

/// @brief Set global Chop Delay
/// @param delay todo:  ms or us ??
void ADS131M0x::setGlobalChopDelay(uint16_t delay) {
  writeRegisterMasked(REG_CFG, delay << 9, REGMASK_CFG_GC_DLY);
}

bool ADS131M0x::setInputChannelSelection(uint8_t channel, uint8_t input) {
  if (channel == 0) {
    writeRegisterMasked(REG_CH0_CFG, input, REGMASK_CHX_CFG_MUX);
    return true;
  } else if (channel == 1) {
    writeRegisterMasked(REG_CH1_CFG, input, REGMASK_CHX_CFG_MUX);
    return true;
  }
#ifndef IS_M02
  else if (channel == 2) {
    writeRegisterMasked(REG_CH2_CFG, input, REGMASK_CHX_CFG_MUX);
    return true;
  } else if (channel == 3) {
    writeRegisterMasked(REG_CH3_CFG, input, REGMASK_CHX_CFG_MUX);
    return true;
  }
#endif
  return false;
}

/// @brief set offset calibration per channel
/// @param channel
/// @param offset
/// @return
bool ADS131M0x::setChannelOffsetCalibration(uint8_t channel, int32_t offset) {

  uint16_t MSB = offset >> 8;
  uint8_t LSB = offset;

  if (channel == 0) {
    writeRegisterMasked(REG_CH0_OCAL_MSB, MSB, 0xFFFF);
    writeRegisterMasked(REG_CH0_OCAL_LSB, LSB << 8, REGMASK_CHX_OCAL0_LSB);
    return true;
  } else if (channel == 1) {
    writeRegisterMasked(REG_CH1_OCAL_MSB, MSB, 0xFFFF);
    writeRegisterMasked(REG_CH1_OCAL_LSB, LSB << 8, REGMASK_CHX_OCAL0_LSB);
    return true;
  }
#ifndef IS_M02
  else if (channel == 2) {
    writeRegisterMasked(REG_CH2_OCAL_MSB, MSB, 0xFFFF);
    writeRegisterMasked(REG_CH2_OCAL_LSB, LSB << 8, REGMASK_CHX_OCAL0_LSB);
    return true;
  } else if (channel == 3) {
    writeRegisterMasked(REG_CH3_OCAL_MSB, MSB, 0xFFFF);
    writeRegisterMasked(REG_CH3_OCAL_LSB, LSB << 8, REGMASK_CHX_OCAL0_LSB);
    return true;
  }
#endif
  return false;
}

/// @brief set gain calibration per channel
/// @param channel
/// @param gain
/// @return
bool ADS131M0x::setChannelGainCalibration(uint8_t channel, uint32_t gain) {

  uint16_t MSB = gain >> 8;
  uint8_t LSB = gain;

  if (channel == 0) {
    writeRegisterMasked(REG_CH0_GCAL_MSB, MSB, 0xFFFF);
    writeRegisterMasked(REG_CH0_GCAL_LSB, LSB << 8, REGMASK_CHX_GCAL0_LSB);
    return true;
  } else if (channel == 1) {
    writeRegisterMasked(REG_CH1_GCAL_MSB, MSB, 0xFFFF);
    writeRegisterMasked(REG_CH1_GCAL_LSB, LSB << 8, REGMASK_CHX_GCAL0_LSB);
    return true;
  }
#ifndef IS_M02
  else if (channel == 2) {
    writeRegisterMasked(REG_CH2_GCAL_MSB, MSB, 0xFFFF);
    writeRegisterMasked(REG_CH2_GCAL_LSB, LSB << 8, REGMASK_CHX_GCAL0_LSB);
    return true;
  } else if (channel == 3) {
    writeRegisterMasked(REG_CH3_GCAL_MSB, MSB, 0xFFFF);
    writeRegisterMasked(REG_CH3_GCAL_LSB, LSB << 8, REGMASK_CHX_GCAL0_LSB);
    return true;
  }
#endif
  return false;
}

/// @brief reset device from register, read CAP 8.5.1.10 Commands from official documentation
/// @param
/// @return True if the device responded with the RSP_RESET_OK message
bool ADS131M0x::resetDevice(void) {
  uint8_t x = 0;
  uint8_t x2 = 0;
  uint16_t ris = 0;

  SPI1.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE1));
  SPI_Demux::select_chip(demuxAddr);
  delayMicroseconds(2);

  x = SPI1.transfer(0x00);
  x2 = SPI1.transfer(0x11);
  SPI1.transfer(0x00);

  ris = ((x << 8) | x2);

  delayMicroseconds(2);
  SPI_Demux::deselect_chip();
  SPI1.endTransaction();

  if (RSP_RESET_OK == ris) {
    return true;
  }
  return false;
}

/// @brief Read ADC port (all Ports)
/// @param
/// @return
adcOutput ADS131M0x::readADC(void) {
  uint8_t x = 0;
  uint8_t x2 = 0;
  uint8_t x3 = 0;
  int32_t aux;
  adcOutput res;

  SPI1.beginTransaction(SPISettings(5000000, MSBFIRST, SPI_MODE1));
  SPI_Demux::select_chip(demuxAddr);

  x = SPI1.transfer(0x00);
  x2 = SPI1.transfer(0x00);
  SPI1.transfer(0x00);

  res.status = ((x << 8) | x2);

  // read CH0 --------
  x = SPI1.transfer(0x00);
  x2 = SPI1.transfer(0x00);
  x3 = SPI1.transfer(0x00);
  aux = (((x << 16) | (x2 << 8) | x3) & 0x00FFFFFF);
  if (aux > 0x7FFFFF) {
    res.ch0 = ((~(aux) & 0x00FFFFFF) + 1) * -1;
  } else {
    res.ch0 = aux;
  }

  // NOTE - THIS SHOULD NOT BE HERE! - Robert Nies
  // // faster!!!
  // SPI1.transfer(0x00);
  // SPI1.transfer(0x00);
  // SPI1.transfer(0x00);

  // read CH1 --------
  x = SPI1.transfer(0x00);
  x2 = SPI1.transfer(0x00);
  x3 = SPI1.transfer(0x00);
  aux = (((x << 16) | (x2 << 8) | x3) & 0x00FFFFFF);
  if (aux > 0x7FFFFF) {
    res.ch1 = ((~(aux) & 0x00FFFFFF) + 1) * -1;
  } else {
    res.ch1 = aux;
  }

#ifndef IS_M02
  x = SPI1.transfer(0x00);
  x2 = SPI1.transfer(0x00);
  x3 = SPI1.transfer(0x00);
  aux = (((x << 16) | (x2 << 8) | x3) & 0x00FFFFFF);
  if (aux > 0x7FFFFF) {
    res.ch2 = ((~(aux) & 0x00FFFFFF) + 1) * -1;
  } else {
    res.ch2 = aux;
  }

  x = SPI1.transfer(0x00);
  x2 = SPI1.transfer(0x00);
  x3 = SPI1.transfer(0x00);
  aux = (((x << 16) | (x2 << 8) | x3) & 0x00FFFFFF);
  if (aux > 0x7FFFFF) {
    res.ch3 = ((~(aux) & 0x00FFFFFF) + 1) * -1;
  } else {
    res.ch3 = aux;
  }
#endif

  SPI1.transfer(0x00);
  SPI1.transfer(0x00);
  SPI1.transfer(0x00);

  SPI_Demux::deselect_chip();
  SPI1.endTransaction();

  return res;
}
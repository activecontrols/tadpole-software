#include "AMT242AV.h"

#define ENCODER_SERIAL Serial8

// max reading for a 12 bit encoder
#define MAX_READING ((1 << 12) - 1)

AMT242AV::AMT242AV(uint8_t DE_RE, uint8_t ID) : DE_RE(DE_RE), ID(ID) {
}

void AMT242AV::begin() {
  ENCODER_SERIAL.begin(2000000);
  digitalWrite(DE_RE, HIGH);
  pinMode(DE_RE, OUTPUT);
}

bool AMT242AV::wait_for_avail(unsigned long long delay_micros = 2500) {
  unsigned long long start_time = micros();
  while (1) {
    if (ENCODER_SERIAL.available())
      return true;
    if (micros() - start_time > delay_micros)
      return false;
    delayMicroseconds(10);
  }
}

bool AMT242AV::_read_pos(uint16_t *out) {
  uint16_t transmission;
  uint16_t pos;
  uint8_t cs_transmission;
  uint8_t cs_real;

  // clear uart receive buffer
  while (ENCODER_SERIAL.available())
    ENCODER_SERIAL.read();

  // switch MAX485 to transmit mode
  digitalWrite(DE_RE, HIGH);
  delayMicroseconds(70);

  // send read position command
  ENCODER_SERIAL.write(ID);
  ENCODER_SERIAL.flush();

  // wait for uart to finish transmission
  //   while (!(ll_uart_intf->ISR & USART_ISR_TC))
  //     ;

  // switch to receive mode
  digitalWrite(DE_RE, LOW);

  uint16_t res = 0;
  if (!wait_for_avail()) {
    goto FAIL;
  }
  res |= ENCODER_SERIAL.read();
  if (!wait_for_avail()) {
    goto FAIL;
  }
  res |= ENCODER_SERIAL.read() << 8;

  digitalWrite(DE_RE, HIGH);

  // lowest 14 bits contain data
  transmission = res & 0b0011111111111111;
  pos = transmission >> 2; // we are using 12 bit encoder, datasheet says to throw out lowest two bits

  // bits 15 and 16 are checksum
  cs_transmission = (res >> 14) & 0b11;

  cs_real = 0;

  // highest bit is for odd-numbered bits, second highest is for even
  // checksums calculated using odd parity
  for (int i = 0; i < 6; ++i) {
    cs_real ^= pos >> (i * 2);
  }

  // odd parity
  cs_real = (~cs_real) & 0b11;

  if (cs_real != cs_transmission)
    goto FAIL;

  *out = pos;

  return true;

// fail condition could be either transmission timed out or checksum failed
FAIL:
  // set MAX485 to inactive state
  digitalWrite(DE_RE, HIGH);
  return false;
}

// read position as a fraction of total range of motion (outputs double between 0.0 and 1.0) (0.0 -> 0 degrees, ..., 1.0 -> 360 degrees)
// this encoder is 12 bit resolution which is approx. 0.1 degree resolution
bool AMT242AV::read_pos(double *out, int max_tries) {
  uint16_t raw_pos;
  for (int i = 0; i < max_tries; ++i) {
    if (_read_pos(&raw_pos)) {
      *out = (double)raw_pos / MAX_READING;
      return true;
    }

    delayMicroseconds(150);
  }

  return false;
}

void AMT242AV::zero() {
  // switch MAX485 to transmit mode
  digitalWrite(DE_RE, HIGH);

  delayMicroseconds(70);

  ENCODER_SERIAL.write(ID | 0x02);
  ENCODER_SERIAL.flush(); // flush doesn't do anything on portenta h7 but maybe on other platforms it will

  // wait for uart to finish transmission
  //   while (!(ll_uart_intf->ISR & USART_ISR_TC))
  //     ;

  digitalWrite(DE_RE, LOW);
}

void AMT242AV::reset() {
  // switch MAX485 to transmit mode
  digitalWrite(DE_RE, HIGH);

  delayMicroseconds(70);

  ENCODER_SERIAL.write(ID | 0x03);
  ENCODER_SERIAL.flush(); // flush doesn't do anything on portenta h7 but maybe on other platforms it will

  // wait for uart to finish transmission
  //   while (!(ll_uart_intf->ISR & USART_ISR_TC))
  //     ;

  digitalWrite(DE_RE, LOW);
}
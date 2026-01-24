#pragma once

#include <Arduino.h>

// library for communicating with an AMT242A-V absolute encoder (over a uart interface -> MAX485 module -> absolute encoder)
class AMT242AV {
public:
  AMT242AV(uint8_t DE_RE, uint8_t ID);
  void begin();

  bool read_pos(double *out, int max_retries = 10);
  void zero();
  void reset();

private:
  uint8_t DE_RE;
  uint8_t ID;

  bool wait_for_avail(unsigned long long);
  bool _read_pos(uint16_t *);
};
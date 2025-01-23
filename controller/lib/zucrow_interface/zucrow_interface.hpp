#ifndef ZUCROW_H
#define ZUCROW_H

#include <Arduino.h>

#define SYNC_PIN 41
#define SYNC_CURVE_RUNNING true
#define SYNC_CURVE_IDLE false

namespace ZucrowInterface {
void begin();
void set_sync_line(bool status);
bool check_fault_from_zucrow();
void send_fault_to_zucrow();
} // namespace ZucrowInterface

#endif
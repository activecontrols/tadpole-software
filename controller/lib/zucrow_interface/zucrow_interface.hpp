#ifndef ZUCROW_H
#define ZUCROW_H

#include <Arduino.h>
#include "MCP48xx.hpp"

// INPUTS FROM ZUCROW
#define ZUCROW_PANIC false
#define ZUCROW_NO_PANIC true

#define ZUCROW_SYNC_RUNNING true
#define ZUCROW_SYNC_IDLE false

// OUTPUTS FROM TEENSY
#define TEENSY_PANIC false
#define TEENSY_NO_PANIC true

#define TEENSY_SYNC_RUNNING true
#define TEENSY_SYNC_IDLE false

namespace ZucrowInterface {

// sets output lines to default states, configures inputs, configures SPI
void begin();

// returns true if zucrow fault line is in fault state
bool check_fault_from_zucrow();

// returns ZUCROW_SYNC_RUNNING or ZUCROW_SYNC_IDLE depending on zucrow sync line state
bool check_sync_from_zucrow();

// sets zucrow panic line to panic state
void send_fault_to_zucrow();

// sets zucrow panic line to non-panic state
void send_ok_to_zucrow();

// send TEENSY_SYNC_RUNNING or TEENSY_SYNC_IDLE to zucrow
void send_sync_to_zucrow(bool status);

// send valve angle telemetry to zucrow, pos should be from 0 to 0.25 for 0 to 90 deg
void send_valve_angles_to_zucrow(float lox_pos, float ipa_pos);

} // namespace ZucrowInterface

#endif
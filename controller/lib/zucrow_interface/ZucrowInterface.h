#ifndef ZUCROW_H
#define ZUCROW_H

// INPUTS FROM ZUCROW
#define ZUCROW_PANIC false
#define ZUCROW_NO_PANIC true

#define ZUCROW_SYNC_RUNNING false
#define ZUCROW_SYNC_IDLE true

// OUTPUTS FROM TEENSY
#define TEENSY_PANIC true
#define TEENSY_NO_PANIC false

#define TEENSY_SYNC_RUNNING true
#define TEENSY_SYNC_IDLE false

namespace ZucrowInterface {

// sets output lines to default states, configures inputs, configures SPI
void begin();

// returns true if zucrow fault line is in fault state
bool check_fault_from_zucrow();

// returns ZUCROW_SYNC_RUNNING or ZUCROW_SYNC_IDLE depending on zucrow sync line state
bool check_sync_from_zucrow();

// sets teensy panic line to panic state
void send_fault_to_zucrow();

// sets teensy panic line to non-panic state
void send_ok_to_zucrow();

// send TEENSY_SYNC_RUNNING or TEENSY_SYNC_IDLE to zucrow
void send_sync_to_zucrow(bool status);

// send valve angle telemetry to zucrow, pos should be from 0 to 0.25 for 0 to 90 deg
void send_valve_angles_to_zucrow(float lox_pos, float ipa_pos);

// for debug
void print_zi_status();

// for AI calib
void zero_angle_outputs();

// for manual movements
void report_angles_for_five_seconds();

} // namespace ZucrowInterface

#endif
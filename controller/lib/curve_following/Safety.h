#ifndef SAFETY_H
#define SAFETY_H

#define CHECK_SERIAL_KILL // should check for 'k' on serial monitor to kill
// #define ENABLE_ZUCROW_SAFETY        // checks for zucrow ok before starting
// #define ENABLE_ODRIVE_SAFETY_CHECKS // check if odrive disconnects or falls behind
#define ENABLE_WC_SAFETY_CHECKS // check window comparator values

#define DONT_KILL 0                     // continue loop
#define KILLED_BY_ZUCROW 1              // zucrow panic line caused kill
#define KILLED_BY_SERIAL 2              // serial cmd caused kill
#define KILLED_BY_ANGLE_OOR 3           // angle compare caused kill
#define ANGLE_OOR_THRESH (10.0 / 360.0) // if angle is 10 degrees off, instantly kill
#define KILLED_BY_ODRIVE_FAULT 4        // odrive exits closed loop control
#define KILLED_BY_WC 5                  // window comparator checks

namespace Safety {
void begin();
void kill_response(int kill_reason);
int check_for_kill();
} // namespace Safety

#endif
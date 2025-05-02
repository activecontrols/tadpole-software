#ifndef SAFETY_H
#define SAFETY_H

#define CHECK_SERIAL_KILL           // should check for 'k' on serial monitor to kill
#define ENABLE_ZUCROW_SAFETY        // checks for zucrow ok before starting
#define ENABLE_ODRIVE_SAFETY_CHECKS // check if odrive disconnects or falls behind
#define ENABLE_WC_SAFETY_CHECKS     // check window comparator values

#define DONT_KILL 0                     // continue loop
#define KILLED_BY_ZUCROW 1              // zucrow panic line caused kill
#define KILLED_BY_SERIAL 2              // serial cmd caused kill
#define KILLED_BY_ANGLE_OOR_LOX 3       // angle compare caused kill
#define KILLED_BY_ANGLE_OOR_IPA 4       // angle compare caused kill
#define ANGLE_OOR_START 3               // start angle OOR checks if t > 3 seconds
#define ANGLE_OOR_THRESH (10.0 / 360.0) // if angle is 10 degrees off, instantly kill
#define KILLED_BY_ODRIVE_FAULT_LOX 5    // odrive exits closed loop control
#define KILLED_BY_ODRIVE_FAULT_IPA 6    // odrive exits closed loop control
#define KILLED_BY_WC 7                  // window comparator checks

namespace Safety {
void begin();
void kill_response(int kill_reason);
int check_for_kill(float time_seconds);
} // namespace Safety

#endif
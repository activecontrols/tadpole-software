/*
 * TadpoleGimbal.hpp
 *
 *  Created on: 2024-10-09 by Robert Nies
 *  Maintained by Robert Nies
 *  Description: This file contains the declaration of the TadpoleGimbal namespace, which provides functions for
 *  controlling the linear actuators.
 */

#ifndef TADPOLE_GIMBAL_H
#define TADPOLE_GIMBAL_H

#include <UltramotionActuator.hpp>
#include <FlexCAN_T4.h>
#include "Router.h"
#define INT_BUFFER_SIZE (50) // for router

#define PRIMARY_CAN_ID 0x00000003   // 0x00000000 to 0x1FFFFFFF (0 x 29 to 1 x 29)
#define SECONDARY_CAN_ID 0x00000004 // 0x00000000 to 0x1FFFFFFF (0 x 29 to 1 x 29)

#define CAN_SPD 1000000 // https://ultramotion.com/servo_cylinder/UM711574_(Servo_Cylinder_CAN_Interface)_Rev_A.01.pdf

#define DECODE_STR_LEN 8
// decode str declared in .cpp

namespace TadpoleGimbal {

void begin();
void set_primary_length();
void set_secondary_length();
void handle_can_msg(const CAN_message_t &msg);

}; // namespace TadpoleGimbal

#endif
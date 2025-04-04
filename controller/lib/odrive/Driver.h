/*
 * Driver.h
 *
 *  Created on: 2024-06-10 by Vincent Palmerio
 *  Maintained by Vincent Palmerio and Ishan Goel
 *  Description: This file contains the declaration of the Driver namespace, which provides functions for
 *  controlling the position and thrust of, and retrieviing info from, the LOX (Liquid Oxygen) and IPA (Isopropyl Alcohol)
 *  ODrive valves in the Tadpole rocket engine.
 */

#ifndef DRIVER_H
#define DRIVER_H

#include "ODrive.h"

namespace Driver {

extern ODrive loxODrive;
extern ODrive ipaODrive;

void begin();

}; // namespace Driver

#endif
#ifndef PRESSURE_H
#define PRESSURE_H

/*
    * Pressure.h
    *
    *  Created on: 2024-6-19 by Vincent Palmerio
    *  Description: This file contains the declaration of the Pressure namespace, 
    *  which contains all declarations of the PressureObj class for easy manageability.
*/

#include "PressureObj.h"

#define LOX_UPSTREAM_PIN 0
#define LOX_UPSTREAM_VOLT_MIN 0
#define LOX_UPSTREAM_VOLT_MAX 10
#define LOX_UPSTREAM_PSI_MIN 0
#define LOX_UPSTREAM_PSI_MAX 100

namespace Pressure {
    
    PressureObj loxUpstreamPressure(
        LOX_UPSTREAM_PIN, LOX_UPSTREAM_VOLT_MIN, 
        LOX_UPSTREAM_VOLT_MAX, LOX_UPSTREAM_PSI_MIN, 
        LOX_UPSTREAM_PSI_MAX
    ); //example pressure sensor obj declaration, may not be used in final code

}

#endif // PRESSURE_H
#include "valve_controller.h"
#include "math.h"

#define GRAVITY_FT_S 32.1740 // Gravity in (ft / s^2)

Venturi water_venturi{.inlet_area = 0.127, .throat_area = 0.062, .cd = 1}; // in^2 for both

float water_density() {
  return 0.036127; // lb/in^3
}

// Estimates mass flow across a venturi using pressure sensor data and fluid information.
float estimate_mass_flow(float pressure_delta, Venturi venturi, float fluid_density) {
  pressure_delta = pressure_delta > 0 ? pressure_delta : 0; // block negative under sqrt
  float area_term = pow(venturi.throat_area / venturi.inlet_area, 2);
  return venturi.throat_area * sqrt(2 * fluid_density * pressure_delta * 12 * GRAVITY_FT_S / (1 - area_term)) * venturi.cd;
}

VC_State log_only(Sensor_Data sensor_data) {
  VC_State vc_state;
  vc_state.measured_water_mdot = estimate_mass_flow(
      sensor_data.water_venturi_upstream_pressure - sensor_data.water_venturi_throat_pressure,
      water_venturi, water_density());
  return vc_state;
}
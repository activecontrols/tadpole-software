#include "valve_controller.h"
#include "math.h"

#define GRAVITY_FT_S 32.1740 // Gravity in (ft / s^2)

Venturi ox_venturi{.inlet_area = 0.127, .throat_area = 0.066, .cd = 1};  // in^2 for both
Venturi ipa_venturi{.inlet_area = 0.127, .throat_area = 0.062, .cd = 1}; // in^2 for both

float water_density() {
  return 0.036127; // lb/in^3
}

// Estimates mass flow across a venturi using pressure sensor data and fluid information.
float estimate_mass_flow(Fluid_Line fluid_line, Venturi venturi, float fluid_density) {
  float pressure_delta = fluid_line.venturi_differential_pressure;
  pressure_delta = pressure_delta > 0 ? pressure_delta : 0; // block negative under sqrt
  float area_term = pow(venturi.throat_area / venturi.inlet_area, 2);
  return venturi.throat_area * sqrt(2 * fluid_density * pressure_delta * 12 * GRAVITY_FT_S / (1 - area_term)) * venturi.cd;
}

VC_State vc_state;
void log_only(Sensor_Data sensor_data) {
  float measured_mass_flow_ox = estimate_mass_flow(sensor_data.ox, ox_venturi, water_density());
  float measured_mass_flow_ipa = estimate_mass_flow(sensor_data.ipa, ipa_venturi, water_density());

  vc_state.measured_lox_mdot = measured_mass_flow_ox;
  vc_state.measured_ipa_mdot = measured_mass_flow_ipa;
}
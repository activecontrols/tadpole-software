#include <valve_controller.hpp>

#define tadpole_AREA_OF_THROAT 1    // TODO - set this
#define tadpole_C_STAR 1            // TODO - set this
#define tadpole_C_F 1               // TODO - set this
#define tadpole_MASS_FLOW_RATIO 1.2 // #ox = 1.2 * fuel

// convert chamber pressure to total mass flow rate
// https://en.wikipedia.org/wiki/Characteristic_velocity
// OUTPUT: total mass flow rate
float c_star_eq(float chamber_pressure) {
  return chamber_pressure * tadpole_AREA_OF_THROAT / tadpole_C_STAR;
}

// convert thrust to chamber pressure
// https://seitzman.gatech.edu/classes/ae4451/thrust_coefficient.pdf
// OUTPUT: chamber pressure
float c_f_eq(float thrust) {
  return thrust / tadpole_C_F / tadpole_AREA_OF_THROAT;
}

// convert total mass flow into OX and FUEL flow rates
// OUTPUT: mass_flow_ox and mass_flow_fuel
void mass_balance(float total_mass_flow, float *mass_flow_ox, float *mass_flow_fuel) {
  *mass_flow_ox = total_mass_flow / (1 + tadpole_MASS_FLOW_RATIO) * tadpole_MASS_FLOW_RATIO;
  *mass_flow_fuel = total_mass_flow / (1 + tadpole_MASS_FLOW_RATIO);
}

// convert mass flow into valve angle
// OUTPUT: valve angle (radians)
float valve_angle(float mass_flow) {
  return mass_flow; // TODO - implement this
}

// get valve angles given thrust
void open_loop_thrust_control(float thrust, float *angle_ox, float *angle_fuel) {
  float mass_flow_ox;
  float mass_flow_fuel;
  mass_balance(c_star_eq(c_f_eq(thrust)), &mass_flow_ox, &mass_flow_fuel);
  *angle_ox = valve_angle(mass_flow_ox);
  *angle_fuel = valve_angle(mass_flow_fuel);
}
#include <valve_controller.hpp>

#define tadpole_AREA_OF_THROAT 1    // TODO - set this
#define tadpole_C_STAR 1            // TODO - set this
#define tadpole_C_F 1               // TODO - set this
#define tadpole_MASS_FLOW_RATIO 1.2 // #ox = 1.2 * fuel

// convert chamber pressure to total mass flow rate
// https://en.wikipedia.org/wiki/Characteristic_velocity
// OUTPUT: total mass flow rate
float mass_flow_rate(float chamber_pressure) {
  return chamber_pressure * tadpole_AREA_OF_THROAT / tadpole_C_STAR;
}

// convert thrust to chamber pressure
// https://seitzman.gatech.edu/classes/ae4451/thrust_coefficient.pdf
// OUTPUT: chamber pressure
float chamber_pressure(float thrust) {
  return thrust / tadpole_C_F / tadpole_AREA_OF_THROAT;
}

// convert total mass flow into OX and FUEL flow rates
// OUTPUT: mass_flow_ox and mass_flow_fuel
void mass_balance(float total_mass_flow, float *mass_flow_ox, float *mass_flow_fuel) {
  *mass_flow_ox = total_mass_flow / (1 + tadpole_MASS_FLOW_RATIO) * tadpole_MASS_FLOW_RATIO;
  *mass_flow_fuel = total_mass_flow / (1 + tadpole_MASS_FLOW_RATIO);
}

// 0.000 corresponds to 0 deg, 4.96 corresponds to 90 deg, each is 9 deg increment
float valve_angle_table[] =
    {0.000, 0.070, 0.161, 0.378, 0.670, 1.000, 1.450, 2.050, 2.780, 3.710, 4.960};

// convert cv into valve angle
// OUTPUT: valve angle (radians)
float valve_angle(float cv) {
  for (int i = 0; i < 10; i++) {
    if (valve_angle_table[i] <= cv && cv < valve_angle_table[i + 1]) {
      return (cv - valve_angle_table[i]) / (valve_angle_table[i + 1] - valve_angle_table[i]) * 9 + (i * 9); // interpolate
    }
  }
  // "Mass flow required is higher than max angle"
  return 90.0 * DEGREES_TO_RAD; // return max angle of 90 degrees
}

// convert mass_flow into valve cv
// OUTPUT: cv (unitless)
float sub_critical_cv(float mass_flow) {
  return 0.0;
}

// get valve angles given thrust
void open_loop_thrust_control(float thrust, float *angle_ox, float *angle_fuel) {
  float mass_flow_ox;
  float mass_flow_fuel;
  mass_balance(mass_flow_rate(chamber_pressure(thrust)), &mass_flow_ox, &mass_flow_fuel);
  *angle_ox = valve_angle(sub_critical_cv(mass_flow_ox));
  *angle_fuel = valve_angle(sub_critical_cv(mass_flow_fuel));
}
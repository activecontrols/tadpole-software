#ifndef PI_CONTROLLER_H
#define PI_CONTROLLER_H

#include <Arduino.h>

class PI_Controller {
public:
  PI_Controller(float kp, float ki, float max_output);
  PI_Controller(float kp, float ki, float min_output, float max_output);
  PI_Controller(float kp, float ki, float min_output, float max_output, float i_limit);
  void reset();
  float compute(float input_error);
  float p_component;
  float i_component;

private:
  float kp;
  float ki;
  float min_output;
  float max_output;
  float i_limit;
  
  long long last_compute_time = -1;
  float err_sum = 0;
};

struct Controller_State {
  float chamber_pressure_controller_p_component;
  float chamber_pressure_controller_i_component;
  float lox_angle_controller_p_component;
  float lox_angle_controller_i_component;
  float ipa_angle_controller_p_component;
  float ipa_angle_controller_i_component;
};

namespace ClosedLoopControllers {
void reset();
Controller_State getState();

extern PI_Controller Chamber_Pressure_Controller;
extern PI_Controller LOX_Angle_Controller;
extern PI_Controller IPA_Angle_Controller;
} // namespace ClosedLoopControllers

#endif
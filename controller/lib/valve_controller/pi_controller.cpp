#include "pi_controller.hpp"
#include <Arduino.h>

PI_Controller::PI_Controller(float kp, float ki) {
  this->kp = kp;
  this->ki = ki;
}

float PI_Controller::compute(float input_error) {
  long long this_compute_time = millis();
  if (this->last_compute_time == -1) {
    this->last_compute_time = this_compute_time; // zeroes out the delta on the first iteration
  }

  this->err_sum += input_error * (this_compute_time - last_compute_time) * 0.001;
  this->last_compute_time = this_compute_time;
  return this->kp * input_error + this->ki * this->err_sum;
}

namespace ClosedLoopControllers {
PI_Controller Chamber_Pressure_Controller(1, 1); // TODO RJN CL - PID constants
PI_Controller LOX_Angle_Controller(1, 1);
PI_Controller IPA_Angle_Controller(1, 1);
} // namespace ClosedLoopControllers

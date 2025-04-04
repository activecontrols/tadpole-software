#include "pi_controller.h"
#include <Arduino.h>

PI_Controller::PI_Controller(float kp, float ki, float max_output) {
  this->kp = kp;
  this->ki = ki;
  this->max_output = max_output;
}

float PI_Controller::compute(float input_error) {
  long long this_compute_time = millis();
  if (this->last_compute_time == -1) {
    this->last_compute_time = this_compute_time; // zeroes out the delta on the first iteration
  }

  this->err_sum += input_error * (this_compute_time - last_compute_time) * 0.001;
  this->last_compute_time = this_compute_time;

  float raw_output = this->kp * input_error + this->ki * this->err_sum;
  if (raw_output > max_output) {
    return max_output; // clamped high
  }
  if (raw_output < -max_output) {
    return -max_output; // clamped low
  }
  return raw_output;
}

void PI_Controller::reset() {
  last_compute_time = -1;
  err_sum = 0;
}

namespace ClosedLoopControllers {
PI_Controller Chamber_Pressure_Controller(1, 1, INFINITY); // TODO RJN CL - PID constants
PI_Controller LOX_Angle_Controller(1, 1, INFINITY);
PI_Controller IPA_Angle_Controller(1, 1, INFINITY);

void reset() {
  Chamber_Pressure_Controller.reset();
  LOX_Angle_Controller.reset();
  IPA_Angle_Controller.reset();
}
} // namespace ClosedLoopControllers

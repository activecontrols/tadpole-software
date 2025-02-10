#ifndef PI_CONTROLLER_H
#define PI_CONTROLLER_H

class PI_Controller {
public:
  PI_Controller(float kp, float ki);
  float compute(float input_error);

private:
  float kp;
  float ki;

  long long last_compute_time = -1;
  float err_sum = 0;
};

namespace ClosedLoopControllers {
extern PI_Controller Chamber_Pressure_Controller;
extern PI_Controller LOX_Angle_Controller;
extern PI_Controller IPA_Angle_Controller;
} // namespace ClosedLoopControllers

#endif
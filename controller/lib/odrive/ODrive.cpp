#include "ODrive.h"

#include "ZucrowInterface.h"
#include <string>

FlexCAN_T4<CAN3, RX_SIZE_256, TX_SIZE_16> can_intf; // can interface

// pass in a handler that sends CAN messages to each odrive
void setup_can(_MB_ptr handler) {
  can_intf.begin();
  can_intf.setBaudRate(CAN_BAUDRATE);
  can_intf.enableMBInterrupts();
  can_intf.onReceive(handler);
}

ODrive::ODrive(uint32_t can_id, char name[4])
    : ODriveCAN(wrap_can_intf(can_intf), can_id) {

  ODriveCAN::onStatus(onHeartbeatCB, &this->odrive_status);

  // crappy way of doing this, bu it is three characters, so it is fine
  this->name[0] = name[0];
  this->name[1] = name[1];
  this->name[2] = name[2];
  this->name[3] = '\0';
}

// Called every time a Heartbeat message arrives from the ODrive
void onHeartbeatCB(Heartbeat_msg_t &msg, void *user_data) {
  ODriveUserData *odrv_user_data = static_cast<ODriveUserData *>(user_data);
  odrv_user_data->last_heartbeat = msg;
  odrv_user_data->received_heartbeat = true;
}

/**
 * Checks if communication with ODrive is available by requesting the current state
 * Runs in a while loop until the ODrive is connected
 */
void ODrive::checkConnection() {
#if (ENABLE_ODRIVE_COMM)
  while (this->odrive_status.received_heartbeat && this->odrive_status.last_heartbeat.Axis_State == AXIS_STATE_UNDEFINED) {
    Router::info("No response from ODrive...");
    delay(100);
  }
  Router::info("Setting odrive to closed loop control...");
  while (this->odrive_status.last_heartbeat.Axis_State != AXIS_STATE_CLOSED_LOOP_CONTROL) {
    ODriveCAN::clearErrors();
    ODriveCAN::setState(AXIS_STATE_CLOSED_LOOP_CONTROL);
    delay(10);
  }
  ODriveCAN::setControllerMode(CONTROL_MODE_POSITION_CONTROL, INPUT_MODE_POS_FILTER);
#endif
}

/**
 * Set the position for the odrive to control flow through valve.
 * @param pos value to be sent to odrive (valid values are from `MIN_ODRIVE_POS` to `MAX_ODRIVE_POS`).
 */
void ODrive::setPos(float pos) {
  posCmd = pos;
  if (pos < MIN_ODRIVE_POS) {
    Router::info_no_newline("Clipping pos to ");
    Router::info(MIN_ODRIVE_POS);
    pos = MIN_ODRIVE_POS;
  }

  if (pos > MAX_ODRIVE_POS) {
    Router::info_no_newline("Clipping pos to ");
    Router::info(MAX_ODRIVE_POS);
    pos = MAX_ODRIVE_POS;
  }
  pos = 0.25 - pos; // invert command send to motor

#if (ENABLE_ODRIVE_COMM)
  ODriveCAN::setPosition(pos);
#endif
}

/**
 * Checks if there are active errors on the ODrive, if so, store the error in activeError and
 * determine if the ODrive is still armed. If the ODrive is disarmed, store the disarm reason
 * and return `ODRIVE_ERROR_DISARMED`. If it is still armed, return `ODRIVE_ERROR`.
 */
int ODrive::checkErrors() {
#if (ENABLE_ODRIVE_COMM)
  Get_Error_msg_t err_msg;
  ODriveCAN::getError(err_msg);
  activeError = err_msg.Active_Errors;
  disarmReason = err_msg.Disarm_Reason;
  if (activeError != 0) {
    return ODRIVE_ACTIVE_ERROR;
  }
#endif
  return ODRIVE_NO_ERROR;
}

void ODrive::printErrors() {
#if (ENABLE_ODRIVE_COMM)
  checkErrors();
  Router::info(std::string(name, 3) + " ODRIVE active error: " + std::to_string(activeError));
  Router::info(std::string(name, 3) + " ODRIVE disarm reason: " + std::to_string(disarmReason));
#endif
}

/**
 * Clears error codes on ODrive and sets disarmReason and activeError variable to 0
 */
void ODrive::clear() {
#if (ENABLE_ODRIVE_COMM)
  ODriveCAN::clearErrors();
#endif
  disarmReason = 0;
  activeError = 0;
}

/**
 * Blinks the LED on the ODrive for 5 seconds.
 */
void ODrive::identify() {
  Router::info("Identifying LOX ODrive for 5 seconds...");
#if (ENABLE_ODRIVE_COMM)
  ODriveCAN::clearErrors(true); // identify = true - this required modifying the library
  delay(5000);
  ODriveCAN::clearErrors(false); // identify = false - this required modifying the library

#endif
  Router::info("Done");
}

/**
 * Returns a CSV string containing the ODrive Telemetry information, in the following format:
 * position,velocity,voltage,current
 */
char *ODrive::getTelemetryCSV() {
  telemetryCSV.clear();

#if (ENABLE_ODRIVE_COMM)
  position = 0.25 - this->last_enc_msg.Pos_Estimate;
  velocity = this->last_enc_msg.Vel_Estimate;
  voltage = this->last_vc_msg.Bus_Voltage;
  current = this->last_amp_msg.Iq_Measured;
  temperature = this->last_temp_msg.Motor_Temperature;

  telemetryCSV << position << "," << velocity << "," << voltage << "," << current;
#else
  telemetryCSV << "pos" << "," << "vel" << ","
               << "V" << "," << "A" << "," << "T";
#endif

  return telemetryCSV.str;
}

/**
 * Returns a string containing the hardware and firmware major and minor versons of the ODrive
 */
char *ODrive::getODriveInfo() {
  odriveInfo.clear();

#if (ENABLE_ODRIVE_COMM)
  Get_Version_msg_t msg;
  ODriveCAN::getVersion(msg);
  hwVersionMajor = msg.Hw_Version_Major;
  hwVersionMinor = msg.Hw_Version_Minor;
  fwVersionMajor = msg.Fw_Version_Major;
  fwVersionMinor = msg.Fw_Version_Minor;

  odriveInfo << "ODrive Hardware Version: " << hwVersionMajor << "." << hwVersionMinor
             << " | Firmware Version: " << fwVersionMajor << "." << fwVersionMinor << " |||";
#endif

  return odriveInfo.str;
}

/**
 * Command for the Router lib to change the position of the ODrive manually.
 */
void ODrive::setPosConsoleCmd() {

  Router::info_no_newline("Angle (0 to 90 degrees)?");
  String posString = Router::read(INT_BUFFER_SIZE);
  Router::info("Response: " + posString);

  float pos = 0.0;
  int result = std::sscanf(posString.c_str(), "%f", &pos);
  if (result != 1) {
    Router::info("Could not convert input to a float, not continuing");
    return;
  }

  pos /= 360;
  if (pos < MIN_ODRIVE_POS || pos > MAX_ODRIVE_POS) {
    Router::info("Position outside defined range in code, not continuing");
    return;
  }

  setPos(pos);
  Router::info("Position set - watch angle readout.");
  ZucrowInterface::report_angles_for_five_seconds();
}

void ODrive::hardStopHoming() { // @ Xander

  float current = 0;

  Router::info("Configuring Hard Stop Homing..."); // print statement

  // WARNING how does odrive decide which direction to move to reach the desired position???????????????? ----desired position is absolute in refdrance to the inital startup position
  // + cw, - ccw --loooking from top of motor
  ODriveCAN::setControllerMode(CONTROL_MODE_VELOCITY_CONTROL, INPUT_MODE_VEL_RAMP);
  // set the control mode
  // input mode config: position filter / velocity ramp / trap trajectory
  // InputMode.VEL_RAMP      //Activate the setpoint filter

  Router::info("Starting Hard Stop Homing Sequence"); // print statement
  delay(500);
  ODriveCAN::setVelocity(-0.1); // velocity [turn/s]

  while (current < 20 && current > -20) { // thresholds must be experimentally determined. will be diffrent depening on needed current to normally move valve
    Get_Iq_msg_t amp_msg;
    ODriveCAN::getCurrents(amp_msg); // update current reading
    current = amp_msg.Iq_Measured;
    Router::info_no_newline("Current: ");
    Router::info(current);
    delay(20);
  }
  ODriveCAN::setVelocity(0); // velocity [turn/s]
  // done to limit standby / hold current
  // ODriveCAN::setParameter("axis0.requested_state", AXIS_STATE_IDLE); //stop talking movement commands
  // ODriveCAN::setParameter("axis0.requested_state", AXIS_STATE_CLOSED_LOOP_CONTROL); //resume taking movement commands
  delay(300);

  Router::info("Home Found"); // print statement
  // set new home
  Get_Encoder_Estimates_msg_t pos_est_msg;
  ODriveCAN::getFeedback(pos_est_msg);
  Router::info_no_newline("current_pos = ");
  Router::info(pos_est_msg.Pos_Estimate);
  ODriveCAN::setAbsolutePosition(-0.01);

  delay(300);

  // This sets odrive back into position control for the setPos command which doesnt do control mode or input mode setup
  // set the control mode
  // input mode config: position filter / velocity ramp / trap trajectory
  // InputMode.POS_FILTER         //Activate the setpoint filter
  ODriveCAN::setControllerMode(CONTROL_MODE_POSITION_CONTROL, INPUT_MODE_POS_FILTER);

  // ODriveCAN::setParameter("axis0.controller.input_pos", 0); // position [turns]

  delay(300);

  Router::info("Home Set");
  kill();
  delay(100);
  enable();
  Router::info("State Reset");
  ZucrowInterface::report_angles_for_five_seconds();
}

void ODrive::kill() {
  ODriveCAN::setState(AXIS_STATE_IDLE); // stop movement attempt
}

void ODrive::enable() {
  ODriveCAN::setState(AXIS_STATE_CLOSED_LOOP_CONTROL); // resume taking movement commands
  ODriveCAN::setControllerMode(CONTROL_MODE_POSITION_CONTROL, INPUT_MODE_POS_FILTER);
}
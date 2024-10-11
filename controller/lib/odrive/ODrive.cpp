#include "ODrive.h"

#include <string>

FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> can_intf; // can interface

// pass in a handler that sends CAN messages to each odrive
void setup_can(_MB_ptr handler) {
  can_intf.begin();
  can_intf.setBaudRate(CAN_BAUDRATE);
  can_intf.setMaxMB(16);
  can_intf.enableFIFO();
  can_intf.enableFIFOInterrupt();
  can_intf.onReceive(handler);
}

ODrive::ODrive(uint32_t can_id, char name[4]) : ODriveCAN(wrap_can_intf(can_intf), can_id),
                                                threadArgs{false, odrive_status} {

  ODriveCAN::onFeedback(onFeedbackCB, &this->odrive_status);
  ODriveCAN::onStatus(onHeartbeatCB, &this->odrive_status);

  // crappy way of doing this, but it is three characters, so it is fine
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

// Called every time a feedback message arrives from the ODrive
void onFeedbackCB(Get_Encoder_Estimates_msg_t &msg, void *user_data) {
  ODriveUserData *odrv_user_data = static_cast<ODriveUserData *>(user_data);
  odrv_user_data->last_feedback = msg;
  odrv_user_data->received_feedback = true;
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
  while (this->odrive_status.received_heartbeat && this->odrive_status.last_heartbeat.Axis_State != AXIS_STATE_CLOSED_LOOP_CONTROL) {
    ODriveCAN::clearErrors();
    ODriveCAN::setState(AXIS_STATE_CLOSED_LOOP_CONTROL);
    delay(10);
  }
#endif
}

/**
 * Checks the `misconfigured` and `reboot_required` flags (from the ODrive API), and if either
 * are true, return an error
 */
int ODrive::checkConfig() {
#if (ENABLE_ODRIVE_COMM)
  // TODO RJN - this doesn't exist in CAN api
  //   misconfigured = ODriveUART::getParameterAsInt("misconfigured");
  //   rebootRequired = ODriveUART::getParameterAsInt("reboot_required");

  misconfigured = false;
  rebootRequired = false;

  if (misconfigured) {
    Router::info(
        "ERROR: " + std::string(name, 3) +
        " ODRIVE IS MISCONFIGURED, WILL NOT ALLOW CURVE FOLLOWING");
    return ODRIVE_MISCONFIGURED;
  }

  if (rebootRequired) {
    Router::info(
        "ERROR: " + std::string(name, 3) +
        " ODRIVE NEEDS TO REBOOT, WILL NOT ALLOW CURVE FOLLOWING");
    return ODRIVE_REBOOT_REQUIRED;
  }
#endif
  return ODRIVE_NO_ERROR;
}

/**
 * Set the position for the odrive to control flow through valve.
 * @param pos value to be sent to odrive (valid values are from `MIN_ODRIVE_POS` to `MAX_ODRIVE_POS`).
 */
void ODrive::setPos(float pos) {
#if (ENABLE_ODRIVE_COMM)
  ODriveCAN::setPosition(pos);
#endif
  posCmd = pos;
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
  if (activeError != 0) {
    if (activeError == ODRIVE_ERROR_DISARMED) {
      Get_Error_msg_t err_msg;
      ODriveCAN::getError(err_msg);
      disarmReason = err_msg.Disarm_Reason;
      return ODRIVE_ERROR_DISARMED;
    }
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

/*
 * Starts the ODrive watchdog thread, and sets the thread handle to this->watchdogThread
 */
void ODrive::startWatchdogThread() {

  threadArgs.threadExecutionFinished = false;

  /*
   * The TeensyThreads library only allows functions with a void return type and a (void*) argument (or int)
   * to run in a thread. If you try to use a different function signature, nothing will happen and the
   * thread will not start, and the program will continue to run anyway.
   */
  this->watchdogThread = new std::thread(&ODrive::watchdogThreadFunc, (void *)&threadArgs);

  threadID = this->watchdogThread->get_id();

  /*
   * The maximum time slice the watchdog thread can run on the CPU. This should be enough time
   * for the thread to complete one iteration. Timing of how long it takes to retrieve ODrive information
   * will be needed
   */
  threads.setTimeSlice(threadID, 20);

  this->watchdogThread->detach();
}

/*
 * The function to be used for the watchdog thread. This function is an infinite loop that
 * feeds the watchdog, checks for active errors, and checks the control state of the ODrive.
 * If there is an active error, or the control state of the ODrive is anything but in closed
 * loop control ( `AXIS_STATE_CLOSE_LOOP_CONTROL` ), then the thread completes execution with an
 * error code.
 *
 * An incorrect control state, in normal conditions, should happen when the ODrives are
 * done following the throttle curve, after which, the ODrive state is set to idle
 * ( `AXIS_STATE_IDLE` ) and the thread completes execution.
 *
 * NOTE: The TeensyThreads library only allows functions with a void return type and a (void*) argument (or int)
 * to run in a thread. If you try to use a different function signature, nothing will happen and the
 * thread will not start, and the program will continue to run anyway.
 */
void ODrive::watchdogThreadFunc(void *castedArgs) {
  volatile struct ThreadArgs *args = static_cast<ThreadArgs *>(castedArgs);

  int activeError = 0;
  int currentState = 0;

  while (true) {

    /*
     * Had to dig through the ODrive firmware to find the watchdog feed command, which was
     * quite annoying
     * https://github.com/odriverobotics/ODrive/blob/6c3cefdeacce600ba4524f4adc5fd634c7bebd18 Firmware/communication/ascii_protocol.cpp#L123
     * `u` is the command for feeding the watchdog
     * `0` is the axis number (there is only one axis on our odrives, which has index `0`)
     *
     * commands must always have a newline character at the end, which serial.println adds
     */
    // args->serial.println("u 0"); // TODO RJN - is watchdog needed for CAN?

    // TODO RJN - get error static issues
    // Get_Error_msg_t err_msg;
    // ODriveCAN::getError(err_msg);
    // activeError = err_msg.Active_Errors;

    // if (activeError != 0) {
    //   args->threadExecutionFinished = true;
    //   return;
    // }

    /*
     * If ODrive is not in closed loop control, then end the thread.
     * This could happen because the teensy is done with throttle curve following
     * and switches the ODrive state, or because of some error
     */
    currentState = args->odrive_status.last_heartbeat.Axis_State;
    if (args->odrive_status.received_heartbeat && currentState != AXIS_STATE_CLOSED_LOOP_CONTROL) {
      args->threadExecutionFinished = true;
      return;
    }

    /*
     * Give back the remaining time on this thread to the next thread
     */
    threads.yield();
  }
  args->threadExecutionFinished = true;

  /*
   * The thread should never reach this point.
   * It should exit gracefully when ODrive state is set to idle by the main thread
   */
  return;
}

/*
 * Sets the ODrive control state to idle ( 'AXIS_STATE_IDLE` ) and waits for the watchdogThread
 * to terminate
 */
void ODrive::terminateWatchdogThread() {
  ODriveCAN::setState(AXIS_STATE_IDLE);

  if (threadArgs.threadExecutionFinished && watchdogThread->joinable()) {
    this->watchdogThread->join();
  }
  free(watchdogThread);
  watchdogThread = NULL;

  Get_Error_msg_t err_msg;
  ODriveCAN::getError(err_msg);
  isArmed = err_msg.Active_Errors == ODRIVE_ERROR_DISARMED;
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
  Get_Encoder_Estimates_msg_t enc_msg;
  ODriveCAN::getFeedback(enc_msg);
  position = enc_msg.Pos_Estimate;
  velocity = enc_msg.Vel_Estimate;

  Get_Bus_Voltage_Current_msg_t vc_msg;
  ODriveCAN::getBusVI(vc_msg);
  voltage = vc_msg.Bus_Voltage;
  current = vc_msg.Bus_Current;

  telemetryCSV << position << "," << velocity << ","
               << voltage << "," << current;
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

  Router::info("Position?");
  String posString = Router::read(INT_BUFFER_SIZE);
  Router::info("Response: " + posString);

  float pos = 0.0;
  int result = std::sscanf(posString.c_str(), "%f", &pos);
  if (result != 1) {
    Router::info("Could not convert input to a float, not continuing");
    return;
  }

  if (pos < MIN_ODRIVE_POS || pos > MAX_ODRIVE_POS) {
    Router::info("Position outside defined range in code, not continuing");
    return;
  }

  setPos(pos);
  Router::info("Position set");
}

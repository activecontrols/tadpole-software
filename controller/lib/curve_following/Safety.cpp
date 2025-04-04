#include "Safety.h"

#include "WindowComparator.h"
#include "ZucrowInterface.h"
#include "Driver.h"
#include "Router.h"

void Safety::begin() {
#ifndef ENABLE_ZUCROW_SAFETY
  Router::info("WARNING! Running without zucrow checks.");
#endif
#ifndef ENABLE_ODRIVE_SAFETY_CHECKS
  Router::info("WARNING! Running without odrive safety.");
#endif
#ifndef ENABLE_WC_SAFETY_CHECKS
  Router::info("WARNING! Running without wc safety.");
#endif
}

void Safety::kill_response(int kill_reason) {
  Driver::loxODrive.setState(AXIS_STATE_IDLE);
  Driver::ipaODrive.setState(AXIS_STATE_IDLE);
  ZucrowInterface::send_fault_to_zucrow();
  Router::info("Fault detected! Curve following terminated, odrives disabled, fault signal sent to Zucrow.");
  Router::info_no_newline("Fault cause #: ");
  Router::info(kill_reason);

  if (kill_reason == KILLED_BY_WC) {
    Router::info_no_newline("Window comparator ");
    Router::info_no_newline(WindowComparators::WC_ERROR.causeID);
    if (WindowComparators::WC_ERROR.causeReason == WC_CAUSE_OVERFLOW) {
      Router::info_no_newline(" overflow ");
      Router::info_no_newline(WindowComparators::WC_ERROR.causeValue);
      Router::info_no_newline(" > ");
      Router::info(WindowComparators::WC_ERROR.compValue);
    } else {
      Router::info_no_newline(" underflow ");
      Router::info_no_newline(WindowComparators::WC_ERROR.causeValue);
      Router::info_no_newline(" < ");
      Router::info(WindowComparators::WC_ERROR.compValue);
    }
  }
}

int Safety::check_for_kill() {
#ifdef ENABLE_ZUCROW_SAFETY
  if (ZucrowInterface::check_fault_from_zucrow()) {
    return KILLED_BY_ZUCROW;
  }
#endif

#ifdef CHECK_SERIAL_KILL
  if (COMMS_SERIAL.available() && COMMS_SERIAL.read() == 'k') {
    return KILLED_BY_SERIAL;
  }
#endif

#ifdef ENABLE_ODRIVE_SAFETY_CHECKS
  if (abs(Driver::loxODrive.position - Driver::loxODrive.getLastPosCmd()) > ANGLE_OOR_THRESH) {
    return KILLED_BY_ANGLE_OOR;
  }
  if (abs(Driver::ipaODrive.position - Driver::ipaODrive.getLastPosCmd()) > ANGLE_OOR_THRESH) {
    return KILLED_BY_ANGLE_OOR;
  }

  if (Driver::loxODrive.odrive_status.last_heartbeat.Axis_State != AXIS_STATE_CLOSED_LOOP_CONTROL) {
    return KILLED_BY_ODRIVE_FAULT;
  }
  if (Driver::ipaODrive.odrive_status.last_heartbeat.Axis_State != AXIS_STATE_CLOSED_LOOP_CONTROL) {
    return KILLED_BY_ODRIVE_FAULT;
  }
#endif

#ifdef ENABLE_WC_SAFETY_CHECKS
  if (WindowComparators::WC_ERROR.isError) {
    return KILLED_BY_WC;
  }
#endif

  return DONT_KILL;
}

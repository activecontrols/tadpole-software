#include "zucrow_interface.hpp"

void ZucrowInterface::begin() {
  pinMode(SYNC_PIN, OUTPUT);
  digitalWrite(SYNC_PIN, SYNC_CURVE_IDLE);
}

void ZucrowInterface::set_sync_line(bool status) {
  digitalWrite(SYNC_PIN, status);
}

bool ZucrowInterface::check_fault_from_zucrow() { // TODO - implement checking fault
  return false;
}

void ZucrowInterface::send_fault_to_zucrow() { // TODO - implement sending fault
}
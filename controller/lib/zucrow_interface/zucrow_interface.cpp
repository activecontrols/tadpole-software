#include "zucrow_interface.hpp"

void ZucrowInterface::begin() {
  pinMode(ZUCROW_PANIC_PIN, INPUT);
  pinMode(ZUCROW_SYNC_PIN, INPUT);
  pinMode(TEENSY_PANIC_PIN, OUTPUT);
  pinMode(TEENSY_SYNC_PIN, OUTPUT);

  digitalWrite(TEENSY_PANIC_PIN, TEENSY_PANIC);
  digitalWrite(TEENSY_SYNC_PIN, TEENSY_SYNC_IDLE);

  // TODO - SPI setup
}

bool ZucrowInterface::check_fault_from_zucrow() {
  return digitalRead(ZUCROW_PANIC_PIN) == ZUCROW_PANIC;
}

bool ZucrowInterface::check_sync_from_zucrow() {
  return digitalRead(ZUCROW_SYNC_PIN);
}

void ZucrowInterface::send_fault_to_zucrow() {
  digitalWrite(TEENSY_PANIC_PIN, TEENSY_PANIC);
}

void ZucrowInterface::send_ok_to_zucrow() {
  digitalWrite(TEENSY_PANIC_PIN, TEENSY_NO_PANIC);
}

void ZucrowInterface::send_sync_to_zucrow(bool status) {
  digitalWrite(TEENSY_SYNC_PIN, status);
}

void ZucrowInterface::send_valve_angles_to_zucrow(float lox_pos, float ipa_pos) {
  // TODO - SPI magic
}
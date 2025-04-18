#include "ZucrowInterface.h"

#include "teensy_pins.h"
#include "MCP48xx.hpp"
#include "Router.h"

MCP4822 dac(SPI_DEVICE_ZUCROW_DAC);

void ZucrowInterface::begin() {
  pinMode(ZUCROW_PANIC_PIN, INPUT);
  pinMode(ZUCROW_SYNC_PIN, INPUT);
  pinMode(TEENSY_PANIC_PIN, OUTPUT);
  pinMode(TEENSY_SYNC_PIN, OUTPUT);

  digitalWrite(TEENSY_PANIC_PIN, TEENSY_PANIC);
  digitalWrite(TEENSY_SYNC_PIN, TEENSY_SYNC_IDLE);

  // The channels are turned off at startup so we need to turn the channel we need on
  dac.turnOnChannelA();
  dac.turnOnChannelB();

  // We configure the channels in low gain (2.5 V max)
  dac.setGainA(MCP4822::Low);
  dac.setGainB(MCP4822::Low);

  dac.updateDAC();

  Router::add({send_fault_to_zucrow, "zi_send_fault"});
  Router::add({send_ok_to_zucrow, "zi_send_ok"});
  Router::add({[&]() { send_sync_to_zucrow(TEENSY_SYNC_RUNNING); }, "zi_send_run"});
  Router::add({[&]() { send_sync_to_zucrow(TEENSY_SYNC_IDLE); }, "zi_send_idle"});
  Router::add({print_zi_status, "zi_status_print"});
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
  int va = lox_pos * 4 * 4096; // remap pos from 0 to 1, then multiply by  4096 for 12 bit resolution
  int vb = ipa_pos * 4 * 4096;
  dac.setVoltageA(va);
  dac.setVoltageB(vb);
  dac.updateDAC();
}

void ZucrowInterface::print_zi_status() {
  Serial.print("Fault (from zucrow)? ");
  if (ZucrowInterface::check_fault_from_zucrow()) {
    Serial.println("yes");
  } else {
    Serial.println("no");
  }

  Serial.print("Sync (from zucrow)? ");
  if (ZucrowInterface::check_sync_from_zucrow() == ZUCROW_SYNC_RUNNING) {
    Serial.println("running");
  } else {
    Serial.println("idle");
  }
}
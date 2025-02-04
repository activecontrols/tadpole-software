#include "zucrow_interface.hpp"
#include "teensy_pins.hpp"

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

  // We configure the channels in High gain
  // It is also the default value so it is not really needed
  dac.setGainA(MCP4822::High);
  dac.setGainB(MCP4822::High);

  dac.updateDAC();
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
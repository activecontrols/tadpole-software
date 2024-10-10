#include <TadpoleGimbal.hpp> // see header info here

FlexCAN_T4<CAN2, RX_SIZE_2, TX_SIZE_16> gimbalCAN;
char decode_str[DECODE_STR_LEN] = {'K', 'L', 'M', 'G', 'H', 'E', 'F', 'Y'};

namespace TadpoleGimbal {

void begin() {
  gimbalCAN.begin();
  gimbalCAN.setBaudRate(CAN_SPD);
  gimbalCAN.enableMBInterrupts();                     // enable interrupts on entire can bus
  gimbalCAN.onReceive(TadpoleGimbal::handle_can_msg); // send all messages to 'handle_can_msg'

  Router::add({TadpoleGimbal::set_primary_length, "set_primary_length"});
  Router::add({TadpoleGimbal::set_secondary_length, "set_secondary_length"});
}

void set_primary_length() {
  Router::info("Length?");
  String lengthSTR = Router::read(INT_BUFFER_SIZE);
  Router::info("Response: " + lengthSTR);

  int length;
  int result = std::sscanf(lengthSTR.c_str(), "%d", &length);
  if (result != 1) {
    Router::info("Could not convert input to an int, not continuing");
    return;
  }

  gimbalCAN.write(prep_CAN_msg(PRIMARY_CAN_ID, length));
}

void set_secondary_length() {
  Router::info("Length?");
  String lengthSTR = Router::read(INT_BUFFER_SIZE);
  Router::info("Response: " + lengthSTR);

  int length;
  int result = std::sscanf(lengthSTR.c_str(), "%d", &length);
  if (result != 1) {
    Router::info("Could not convert input to an int, not continuing");
    return;
  }

  gimbalCAN.write(prep_CAN_msg(SECONDARY_CAN_ID, length));
}

// TODO - deal with status messages across different acutators
void handle_can_msg(const CAN_message_t &msg) {
  Serial.print("New CAN msg from ID: ");
  Serial.println(msg.id, HEX);
  parse_CAN_frame(msg.buf, msg.len, decode_str, DECODE_STR_LEN);
  Serial.println();
}

} // namespace TadpoleGimbal
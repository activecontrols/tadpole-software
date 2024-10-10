#include <UltramotionActuator.hpp>

const char *status_codes[STATUS_CODE_COUNT] = {
    "Position at or beyond retracted physical stop rPos", // 0 - 8
    "Position at or beyond extended physical stop ePos",
    "Position beyond retracted software limit spMin",
    "Position beyond extended software limit spMax",
    "Supply voltage low, motor in COAST (<6.75 VDC, 1 V hysteresis)",
    "Supply voltage high, motor in dynamic brake  (>44.0 VDC, 2 V hysteresis)",
    "Torque output greater than ovTorq limit",
    "Torque command at maxTorq limit",
    "Speed below “stop” threshold", // 9 - 16
    "Direction is extend",
    "Position at target (position near target within posWin for posTime)",
    "Following error (position error larger than fErrWin for time period fErrTime)",
    "Command RX error (message not received in [rxTO * 800 µs])",
    "Telemetry TX error (message not sent over full telemetry interval)",
    "CAN position command input capped at low limit pMin",
    "CAN position command input capped at upper limit pMax",
    "Trajectory move active ", // 17 - 24
    "Heating active",
    "Temperature at PCB greater than ovTemp value",
    "Temperature at PCB less than unTemp value",
    "Relative humidity at PCB greater than ovHumi value",
    "Fatal error in CONFIG.TXT or HARDWARE.TXT",
    "Fault output bit of DRV8323RS (Bridge Driver)",
    "Erroneous warm reset of the CPU has occurred",
    "opMode (CLI = 0, CAN = 1)", // 25 - 32
    "Interpolation enabled ",
    "Heating enabled ",
    "CAN bus module in passive mode ",
    "USB connected",
    "Opto input 1",
    "Opto input 2",
    "Opto input 3"};

uint32_t current_status = 0; // limitation - don't mix latched status with normal status for the same variable
uint32_t current_status_latched_high = 0;
uint32_t current_status_latched_low = 0;

void reset_status_state() {
  current_status = 0;
  current_status_latched_high = 0;
  current_status_latched_low = 0;
}

void print_new_status_codes(uint32_t new_status, uint32_t old_status) {
  uint32_t status_dif = new_status ^ old_status;

  Serial.println("  New Status Messages: ");
  uint32_t current_status_shift = new_status;
  uint32_t status_dif_shift = status_dif;
  for (int i = 0; i < STATUS_CODE_COUNT; i++) {
    if (status_dif_shift & 0x1 && current_status_shift & 0x1) { // select bits, check if message is NEW and ACTIVE
      Serial.print("    ");
      Serial.println(status_codes[i]);
    };
    status_dif_shift = status_dif_shift >> 1;
    current_status_shift = current_status_shift >> 1;
  }
  Serial.println("  END");

  Serial.println("\n  Cleared Status: ");
  current_status_shift = new_status;
  status_dif_shift = status_dif;
  for (int i = 0; i < STATUS_CODE_COUNT; i++) {
    if (status_dif_shift & 0x1 && !(current_status_shift & 0x1)) { // select bits, check if message is NEW and INACTIVE
      Serial.print("    ");
      Serial.println(status_codes[i]);
    };
    status_dif_shift = status_dif_shift >> 1;
    current_status_shift = current_status_shift >> 1;
  }
  Serial.println("  END");
}

void parse_CAN_byte(uint8_t can_msg, char decode_str, telem *current_telem_frame) {
  // Serial.print("Decoding: ");
  // Serial.print(can_msg);
  // Serial.print(" with ");
  // Serial.println(decode_str);

  uint16_t can_msg_16bit = can_msg;
  uint32_t can_msg_32bit = can_msg;

  if (decode_str == 'A') {
    current_telem_frame->status_word += can_msg_32bit;
  } else if (decode_str == 'B') {
    current_telem_frame->status_word += can_msg_32bit << 8;
  } else if (decode_str == 'C') {
    current_telem_frame->status_word += can_msg_32bit << 16;
  } else if (decode_str == 'D') {
    current_telem_frame->status_word += can_msg_32bit << 24;
  } else if (decode_str == 'E') {
    current_telem_frame->avg_motor_current += can_msg_16bit;
  } else if (decode_str == 'F') {
    current_telem_frame->avg_motor_current += can_msg_16bit << 8;
  } else if (decode_str == 'G') {
    current_telem_frame->abs_servo_cylinder_pos += can_msg_16bit;
  } else if (decode_str == 'H') {
    current_telem_frame->abs_servo_cylinder_pos += can_msg_16bit << 8;
  } else if (decode_str == 'I') {
    current_telem_frame->rel_servo_cylinder_pos += can_msg_16bit;
  } else if (decode_str == 'J') {
    current_telem_frame->rel_servo_cylinder_pos += can_msg_16bit << 8;
  } else if (decode_str == 'K') {
    current_telem_frame->latch_high_status_word += can_msg_32bit;
  } else if (decode_str == 'L') {
    current_telem_frame->latch_high_status_word += can_msg_32bit << 8;
  } else if (decode_str == 'M') {
    current_telem_frame->latch_high_status_word += can_msg_32bit << 16;
  } else if (decode_str == 'N') {
    current_telem_frame->latch_high_status_word += can_msg_32bit << 24;
  } else if (decode_str == 'O') {
    current_telem_frame->latch_low_status_word += can_msg_32bit;
  } else if (decode_str == 'P') {
    current_telem_frame->latch_low_status_word += can_msg_32bit << 8;
  } else if (decode_str == 'Q') {
    current_telem_frame->latch_low_status_word += can_msg_32bit << 16;
  } else if (decode_str == 'R') {
    current_telem_frame->latch_low_status_word += can_msg_32bit << 24;
  } else if (decode_str == 'S') {
    current_telem_frame->phys_stop_pos = can_msg;
  } else if (decode_str == 'T') {
    current_telem_frame->motor_current_avg_16 = can_msg;
  } else if (decode_str == 'U') {
    current_telem_frame->bus_voltage = can_msg;
  } else if (decode_str == 'V') {
    current_telem_frame->motor_current_avg = can_msg;
  } else if (decode_str == 'W') {
    current_telem_frame->max_motor_current_8_bit = can_msg;
  } else if (decode_str == 'X') {
    current_telem_frame->signed_PCB_temp_sensor = can_msg;
  } else if (decode_str == 'Y') {
    current_telem_frame->unsigned_PCB_temp_sensor = can_msg;
  } else if (decode_str == 'Z') {
    current_telem_frame->PCB_relative_humidity = can_msg;
  } else if (decode_str == 'm') {
    current_telem_frame->max_motor_current_16_bit += can_msg_16bit;
  } else if (decode_str == 'c') {
    current_telem_frame->max_motor_current_16_bit += can_msg_16bit << 8;
  } else if (decode_str == 'p') {
    current_telem_frame->unitID += can_msg_32bit;
  } else if (decode_str == 'q') {
    current_telem_frame->unitID += can_msg_32bit << 8;
  } else if (decode_str == 'r') {
    current_telem_frame->unitID += can_msg_32bit << 16;
  } else if (decode_str == 's') {
    current_telem_frame->unitID += can_msg_32bit << 24;
  } else if (decode_str == 't') {
    current_telem_frame->target_pos += can_msg_16bit;
  } else if (decode_str == 'u') {
    current_telem_frame->target_pos += can_msg_16bit << 8;
  } else {
    Serial.print("FATAL ERROR - CAN frame contained non-decodable char: ");
    Serial.println(decode_str);
  }
}

void print_can_data(char decode_str, telem *current_telem_frame) {
  switch (decode_str) {
  case 'A':
  case 'B':
  case 'C':
  case 'D':
    if (!current_telem_frame->has_printed_status) {
      Serial.println("Status: ");
      print_new_status_codes(current_telem_frame->status_word, current_status);
      current_status = current_telem_frame->status_word;
    }
    current_telem_frame->has_printed_status = true;
    break;
  case 'E': // prevent dup
  case 'G':
  case 'I':
  case 'm':
  case 't':
    break;
  case 'F': // and E
    Serial.print("Average motor current over telemetry interval: ");
    Serial.println(current_telem_frame->avg_motor_current);
    break;
  case 'H': // and G
    Serial.print("Servo Cylinder position, absolute encoder value: ");
    Serial.println(current_telem_frame->abs_servo_cylinder_pos);
    break;
  case 'J': // and I
    Serial.print("Position converted to input range (pMin to pMax): ");
    Serial.println(current_telem_frame->rel_servo_cylinder_pos);
    break;
  case 'K':
  case 'L':
  case 'M':
  case 'N':
    if (!current_telem_frame->has_printed_high_status) {
      Serial.println("Status (Latched High): ");
      print_new_status_codes(current_telem_frame->latch_high_status_word, current_status_latched_high);
      current_status_latched_high = current_telem_frame->latch_high_status_word;
    }
    current_telem_frame->has_printed_high_status = true;
    break;
  case 'O':
  case 'P':
  case 'Q':
  case 'R':
    if (!current_telem_frame->has_printed_low_status) {
      Serial.println("Status (Latched Low): ");
      print_new_status_codes(current_telem_frame->latch_low_status_word, current_status_latched_low);
      current_status_latched_low = current_telem_frame->latch_low_status_word;
    }
    current_telem_frame->has_printed_low_status = true;
    break;
  case 'S':
    Serial.print("8-bit position between physical stops (rPos to ePos): ");
    Serial.println(current_telem_frame->phys_stop_pos);
    break;
  case 'T':
    Serial.print("8-bit motor current 16-sample average of last 16 ms: ");
    Serial.println(current_telem_frame->motor_current_avg_16);
    break;
  case 'U': {
    Serial.print("8-bit bus voltage 0 VDC to +50 VDC: ");
    float voltage = current_telem_frame->bus_voltage;
    Serial.print(voltage * 50 / 255);
    Serial.println(" VDC");
    break;
  }
  case 'V':
    Serial.print("8-bit average motor current over telemetry interval: ");
    Serial.println(current_telem_frame->motor_current_avg);
    break;
  case 'W':
    Serial.print("8-bit max motor current over telemetry interval: ");
    Serial.println(current_telem_frame->max_motor_current_8_bit);
    break;
  case 'X':
    Serial.print("8-bit signed integer PCB temp sensor C (-50 to +127): ");
    Serial.println(current_telem_frame->signed_PCB_temp_sensor);
    break;
  case 'Y': {
    Serial.print("8-bit unsigned PCB temp sensor: ");
    int pcb_temp = current_telem_frame->unsigned_PCB_temp_sensor;
    Serial.print(pcb_temp - 50);
    Serial.println(" deg C");
    break;
  }
  case 'Z':
    Serial.print("8-bit PCB relative humidity: ");
    Serial.print(current_telem_frame->PCB_relative_humidity);
    Serial.println("%");
    break;
  case 'c': // and m
    Serial.print("Max motor current over telemetry interval: ");
    Serial.println(current_telem_frame->max_motor_current_16_bit);
    break;
  case 'p':
  case 'q':
  case 'r':
  case 's':
    if (!current_telem_frame->has_printed_ID) {
      Serial.print("ID: 0x");
      Serial.println(current_telem_frame->unitID, HEX);
    }
    current_telem_frame->has_printed_ID = true;
    break;
  case 'u': // and t
    Serial.print("Target position, absolute encoder value: ");
    Serial.println(current_telem_frame->target_pos);
    break;
  default:
    Serial.print("FATAL ERROR - CAN frame contained non-decodable char: ");
    Serial.println(decode_str);
    break;
  }
}

void parse_CAN_frame(const uint8_t can_msg[], uint8_t msg_length, char decode_str[], uint8_t decode_length) {
  if (msg_length != decode_length) {
    Serial.println("FATAL ERROR - CAN frame and decode code lengths do not match!");
  }

  if (msg_length > 8) {
    Serial.println("FATAL ERROR - CAN frame longer than max!");
  }

  telem current_telem_frame; // values are all set to 0

  for (uint8_t i = 0; i < msg_length; i++) {
    parse_CAN_byte(can_msg[i], decode_str[i], &current_telem_frame);
  }

  Serial.println("CAN FRAME START ++++++++++++++++++++++++++");
  for (uint8_t i = 0; i < msg_length; i++) {
    print_can_data(decode_str[i], &current_telem_frame);
  }
  Serial.println("CAN FRAME END  ---------------------------\n");
}

CAN_message_t prep_CAN_msg(uint32_t id, uint16_t target_pos) {
  CAN_message_t msg;
  msg.flags.extended = true;
  msg.id = id;
  msg.len = 2;
  msg.buf[0] = target_pos;
  msg.buf[1] = target_pos >> 8;
  return msg;
}

CAN_message_t prep_CAN_msg(uint32_t id, uint16_t target_pos, uint16_t max_torque) {
  CAN_message_t msg;
  msg.id = id;
  msg.len = 4;
  msg.buf[0] = target_pos;
  msg.buf[1] = target_pos >> 8;
  msg.buf[2] = max_torque;
  msg.buf[3] = max_torque >> 8;
  return msg;
}
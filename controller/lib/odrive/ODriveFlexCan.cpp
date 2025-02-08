#include "ODriveFlexCAN.hpp"

void onReceive(const CAN_message_t& msg, ODriveCAN& odrive) {
    odrive.onReceive(msg.id | (msg.flags.extended ? 0x80000000 : 0), msg.len, msg.buf);
}

void pumpEvents(FlexCAN_T4_Base& can_intf) {
    can_intf.events();
}
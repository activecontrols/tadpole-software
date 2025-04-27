#pragma once

#include "ODriveCAN.h"
#include "FlexCAN_T4.h"

using CanMsg = CAN_message_t;

static bool sendMsg(FlexCAN_T4_Base& can_intf, uint32_t id, uint8_t length, const uint8_t* data) {
    CAN_message_t teensy_msg = {
        .id = id & 0x1fffffff,
        .flags = {.extended = (bool)(id & 0x80000000), .remote = !data},
        .len = length,
    };
    
    if (data) {
        memcpy(teensy_msg.buf, data, length);
    }

    can_intf.events();

    return (can_intf.write(teensy_msg) > 0);
}

void onReceive(const CAN_message_t& msg, ODriveCAN& odrive);
void pumpEvents(FlexCAN_T4_Base& can_intf);

CREATE_CAN_INTF_WRAPPER(FlexCAN_T4_Base)

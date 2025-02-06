#ifndef DIGI_POT_H
#define DIGI_POT_H

#include <Arduino.h>

// structure is 4 addr bits, 2 cmd bits, 10 data bits
// V_WP = volatile wiper (resets on boot, effect is instant)
// NV_WP = non-volatile wiper (restores to this value on boot)

#define ADDR_V_WP_0 0x0000;
#define ADDR_V_WP_1 0x1000;
#define ADDR_NV_WP_0 0x2000;
#define ADDR_NV_WP_1 0x3000;
#define ADDR_V_WP_2 0x6000;
#define ADDR_V_WP_3 0x7000;
#define ADDR_NV_WP_2 0x8000;
#define ADDR_NV_WP_3 0x9000;

#define CMD_WRITE 0x0000;   // write value to register or pot
#define CMD_READ 0x0C00;    // read value from register or pot
#define CMD_INC_DIS 0x0400; // increments wiper setting / disables wiper lock
#define CMD_DEC_EN 0x0800;  // decrements wiper setting / enables wiper lock

class DigiPot {

private:
  int demuxAddr;

public:
  DigiPot(int demuxAddr);
  uint16_t send_cmd(uint16_t addr, uint16_t cmd, uint16_t data);
};

// TODO RJN DigiPot - configure namespace
// TODO RJN TC - make TC code

#endif
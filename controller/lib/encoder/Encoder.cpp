#include "Encoder.h"
#include "Router.h"

namespace Encoder {
AMT242AV encoder(36, 0x54);

void begin() {
  encoder.begin();
  Router::add({zero, "zero_encoder"});
  Router::add({print_pos, "print_encoder"});
}

void print_pos() {
  double pos;
  bool state = encoder.read_pos(&pos);
  Router::info_no_newline("Encoder pos: ");

  Router::info(pos);
  Router::info_no_newline("Encoder error: ");
  Router::info(state);
}

void zero() {
  encoder.zero();
}

} // namespace Encoder

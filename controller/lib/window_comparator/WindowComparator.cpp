#include "WindowComparator.h"

WindowComparator::WindowComparator(int wc_id, float min_v, float max_v) {
  this->wc_id = wc_id;
  this->min_v = min_v;
  this->max_v = max_v;
}

void WindowComparator::check(float v) {
  if (v < min_v) {
    WindowComparators::WC_ERROR.isError = true;
    WindowComparators::WC_ERROR.causeID = wc_id;
    WindowComparators::WC_ERROR.causeReason = WC_CAUSE_UNDERFLOW;
    WindowComparators::WC_ERROR.causeValue = v;
    WindowComparators::WC_ERROR.compValue = min_v;
  }
  if (v > max_v) {
    WindowComparators::WC_ERROR.isError = true;
    WindowComparators::WC_ERROR.causeID = wc_id;
    WindowComparators::WC_ERROR.causeReason = WC_CAUSE_OVERFLOW;
    WindowComparators::WC_ERROR.causeValue = v;
    WindowComparators::WC_ERROR.compValue = max_v;
  }
}

namespace WindowComparators {
wc_error_info WC_ERROR = {.isError = false};
void reset() {
  WC_ERROR.isError = false;
}

// TODO RJN - set these redlines
WindowComparator lox_valve_upstream_pressure(WC_LOX_VALVE_UPSTREAM_ID, -3000, 3000);
WindowComparator lox_valve_downstream_pressure(WC_LOX_VALVE_DOWNSTREAM_ID, -3000, 3000);
WindowComparator lox_venturi_differential_pressure(WC_LOX_VENTURI_DIFFERENTIAL_ID, -3000, 3000);
WindowComparator lox_venturi_temperature(WC_LOX_VENTURI_TEMPERATURE, -3000, 3000);
WindowComparator lox_valve_temperature(WC_LOX_VALVE_TEMPERATURE, -3000, 3000);

WindowComparator ipa_valve_upstream_pressure(WC_IPA_VALVE_UPSTREAM_ID, -3000, 3000);
WindowComparator ipa_valve_downstream_pressure(WC_IPA_VALVE_DOWNSTREAM_ID, -3000, 3000);
WindowComparator ipa_venturi_differential_pressure(WC_IPA_VENTURI_DIFFERENTIAL_ID, -3000, 3000);

WindowComparator chamber_pressure(WC_CHAMBER_PRESSURE_ID, -3000, 3000);

} // namespace WindowComparators

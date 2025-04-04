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

WindowComparator lox_tank_pressure(WC_LOX_TANK_PRESSURE_ID, -1, 2000);
WindowComparator lox_venturi_upstream_pressure(WC_LOX_VENTURI_UPSTREAM_PRESSURE_ID, -1, 2000);
WindowComparator lox_venturi_throat_pressure(WC_LOX_VENTURI_THROAT_PRESSURE_ID, -1, 2000);
WindowComparator lox_venturi_temperature(WC_LOX_VENTURI_TEMPERATURE, -1, 2000);
WindowComparator lox_valve_temperature(WC_LOX_VALVE_TEMPERATURE, -1, 2000);

WindowComparator ipa_tank_pressure(WC_IPA_TANK_PRESSURE_ID, -1, 2000);
WindowComparator ipa_venturi_upstream_pressure(WC_IPA_VENUTRI_UPSTREAM_PRESSURE_ID, -1, 2000);
WindowComparator ipa_venturi_throat_pressure(WC_IPA_VENUTRI_THROAT_PRESSURE_ID, -1, 2000);

WindowComparator chamber_pressure(WC_CHAMBER_PRESSURE_ID, -1, 2000);

} // namespace WindowComparators

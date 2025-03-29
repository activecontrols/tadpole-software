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

WindowComparator example_wc(EXAMPLE_WC_ID, 10, 20); // id, min, max

} // namespace WindowComparators

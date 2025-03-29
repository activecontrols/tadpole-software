#ifndef WINDOW_COMPARATOR_H
#define WINDOW_COMPARATOR_H

#define WC_CAUSE_UNDERFLOW false
#define WC_CAUSE_OVERFLOW true

struct wc_error_info {
  bool isError;     // true if any wc triggers error
  int causeID;      // wc id that caused error
  bool causeReason; // WC_CAUSE_*
  float causeValue; // value that caused error
  float compValue;  // value that was compared against to cause error
};

class WindowComparator {

private:
  int wc_id;
  float min_v;
  float max_v;

public:
  WindowComparator(int wc_id, float min_v, float max_v);
  void check(float v);
};

namespace WindowComparators {

extern wc_error_info WC_ERROR;

#define EXAMPLE_WC_ID 1
extern WindowComparator example_wc;

} // namespace WindowComparators

#endif // WINDOW_COMPARATOR_H
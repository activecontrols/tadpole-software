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

#define WC_WATER_TANK_PRESSURE_ID 1
#define WC_WATER_VENTURI_UPSTREAM_PRESSURE_ID 2
#define WC_WATER_VENTURI_THROAT_PRESSURE_ID 3
#define WC_WATER_TEMPERATURE 4

extern wc_error_info WC_ERROR;

extern WindowComparator water_tank_pressure;
extern WindowComparator water_venturi_upstream_pressure;
extern WindowComparator water_venturi_throat_pressure;
extern WindowComparator water_temperature;
} // namespace WindowComparators

#endif // WINDOW_COMPARATOR_H
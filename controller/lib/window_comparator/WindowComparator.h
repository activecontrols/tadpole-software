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

void reset();

#define WC_LOX_TANK_PRESSURE_ID 1
#define WC_LOX_VENTURI_UPSTREAM_PRESSURE_ID 2
#define WC_LOX_VENTURI_THROAT_PRESSURE_ID 3
#define WC_LOX_VENTURI_TEMPERATURE 4
#define WC_LOX_VALVE_TEMPERATURE 5

#define WC_IPA_TANK_PRESSURE_ID 6
#define WC_IPA_VENUTRI_UPSTREAM_PRESSURE_ID 7
#define WC_IPA_VENUTRI_THROAT_PRESSURE_ID 8

#define WC_CHAMBER_PRESSURE_ID 9

extern wc_error_info WC_ERROR;

extern WindowComparator lox_tank_pressure;
extern WindowComparator lox_venturi_upstream_pressure;
extern WindowComparator lox_venturi_throat_pressure;
extern WindowComparator lox_venturi_temperature;
extern WindowComparator lox_valve_temperature;

extern WindowComparator ipa_tank_pressure;
extern WindowComparator ipa_venturi_upstream_pressure;

extern WindowComparator ipa_venturi_throat_pressure;

extern WindowComparator chamber_pressure;

} // namespace WindowComparators

#endif // WINDOW_COMPARATOR_H
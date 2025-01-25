#define CURRENT_CURVEH_VERSION 2 // UPDATE THIS BIT IF THE STRUCT IS CHANGED - it will invalidate files created in older version

typedef struct {
  float time;      // seconds since start
  float lox_angle; // 0-90 degrees?
  float ipa_angle; // 0-90 degrees?
} lerp_point_angle;

typedef struct {
  float time;   // seconds since start
  float thrust; // 0-100
} lerp_point_thrust;

typedef struct {
  int version = CURRENT_CURVEH_VERSION;
  char curve_label[50]; // max 49 char string label
  bool is_thrust;       // true if thrust, false if angle
  int num_points;
} curve_header;

// NOT USED CURRENTLY
typedef struct {
  float lox_mdot_gains[30];
  float ipa_mdot_gains[30];
  float thrust_gains[30];
  float cf,  // thrust coefficient
      cstar, // characteristic velocity
      cf_efficiency,
      cstar_efficiency,
      cd_ox, // venturi discharge coeff
      cd_ipa,
      at_ox_venturi,  // throat area
      at_ipa_venturi, // throat area
      at_engine;      // tadpole throat area
  // density vs temp. range is 0-29 degrees C
  float rho_ox_by_temp[30];
  // density vs temp. range is 0-29 degrees C
  float rho_ipa_by_temp[30];
  float lox_vapor_press_by_temp[30];
} control_config;
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

enum class curve_type {
    sine,
    chirp,
    lerp,
};

typedef struct {
    float time;      // seconds since start
    float lox_angle; // 0-90 degrees?
    float ipa_angle; // 0-90 degrees?
} lerp_point_open;

typedef struct {
    float time;      // seconds since start
    float thrust;    // 0-100
} lerp_point_closed;

typedef struct {
    char curve_label[50]; // max 49 char string label
    curve_type ctype;
    bool is_open;
    union { // which of these is used depends on ctype
        struct {
            float ipa_amplitude;
            float ipa_period;
            int ipa_num_cycles;
            int ipa_mix_ratio; // one side of mixture ratio (ie 70:30 for oxidizer to fuel, where 30 is ipa_mixture_ratio)
        } sine_open;
        struct {
            float amplitude;
            float period;
            int num_cycles;
        } sine_closed;
        struct {
            float amplitude;
            float start;
            float end;
        } chirp;
        struct {
            int num_points;
            char checksum[4];
        } lerp;
    };
} curve_header;

// all placeholders for now
typedef struct {
    float lox_mdot_gains[30];
    float ipa_mdot_gains[30];
    float thrust_gains[30];
    float cf, // thrust coefficient
    cstar, // characteristic velocity
    cf_efficiency,
    cstar_efficiency,
    cd_ox, // venturi discharge coeff
    cd_ipa,
    at_ox_venturi, // throat area
    at_ipa_venturi, // throat area
    at_engine; // tadpole throat area
    // density vs temp. range is 0-29 degrees C
    float rho_ox_by_temp[30];
    // density vs temp. range is 0-29 degrees C
    float rho_ipa_by_temp[30];
    float lox_vapor_press_by_temp[30];
} control_config;
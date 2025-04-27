shared_settings = {
	"axis0.controller.config.pos_gain": 5,
	"axis0.controller.config.vel_gain": 1,
	"axis0.controller.config.vel_integrator_gain": 2,
	"axis0.controller.config.vel_limit": 5,

	"axis0.controller.config.input_filter_bandwidth": 1000.0, # frequency pos updates will be commanded
	"axis0.controller.config.vel_ramp_rate": 10.0, # called vel ramp limit in GUI

	"axis0.config.motor.current_slew_rate_limit": 100.0, # prevent over current

	"config.brake_resistor0.dc_bus_voltage_feedback_ramp_end": 15.0,
	"config.brake_resistor0.dc_bus_voltage_feedback_ramp_start": 13.0,
	"config.dc_bus_overvoltage_trip_level": 16,
	"config.dc_bus_undervoltage_trip_level": 10.5,
	"can.config.baud_rate": 500000
}

def update_json(fname):
	with open("odrive_conf/" + fname) as f:
		lines = f.readlines()

	for i, line in enumerate(lines):
		for setting, value in shared_settings.items():
			if f'"{setting}"' in line:
				lines[i] = f'  "{setting}": {value},\n'
	
	with open("odrive_conf/" + fname, "w+") as f:
		f.writelines(lines)

update_json("lox.json")
update_json("ipa.json")
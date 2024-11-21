# Tadpole Software: Open Loop Valves

To run test:
 - When code boots it will throw errors if it can't find odrive over serial
   - LOX_ODrive_Serial is Serial1 [Driver.cpp](controller/lib/odrive/Driver.cpp)
   - IPA_ODrive is disabled right now
 - Type `help` for a list of valid router commands
 - Use `set_lox_odrive_pos` to test moving the motor
   - Send a value between -1 and 1 when prompted
 - Use `set_thrust_open_loop` to test setting thrust
   - Send a value between 0 and 600 when prompted

Bonus:
 - Use `load_curve_serial` or `load_curve_sd` to load a thrust curve (sending filename as needed)
 - Use `follow_curve` to execute the curve
 - See [curve_writer.cpp](curve_writer/curve_writer.cpp) for creating curves
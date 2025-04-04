## Router Commands
x = lox/ipa

| Command               | Module        | Function                                                                                |
| --------------------- | ------------- | --------------------------------------------------------------------------------------- |
| help                  | Router        | prints all commands                                                                     |
| ping                  | Router        | prints pong (connection check)                                                          |
| auto_seq              | CurveFollower | runs the auto sequence (waits for zucrow, then follows AUTOCUR and writes to AUTOL.csv) |
| follow_curve          | CurveFollower | follows the currently loaded curve file                                                 |
| load_curve_sd         | Loader        | loads a curve from the sd card                                                          |
| load_curve_serial     | Loader        | loads a curve over serial, called by curvewriter.cpp                                    |
| write_curve_sd        | Loader        | saves the currently loaded curve to a file                                              |
| ls                    | SDCard        | list files                                                                              |
| rm                    | SDCard        | remove a file                                                                           |
| cat                   | SDCard        | prints file contents                                                                    |
| print_sensors         | *             | prints readings from all connected sensors                                              |
| kill                  | Driver        | moves odrives out of closed loop mode                                                   |
| get_odrive_info       | Driver        | prints odrive info to confirm connection                                                |
| clear_x_odrive_errors | Driver        | resets odrive error state                                                               |
| set_x_odrive_pos      | Driver        | manually move odrive                                                                    |
| get_x_cmd_pos         | Driver        | get last command pos sent to odrive                                                     |
| identify_x_odrive     | Driver        | blinks light on corresponding odrive                                                    |
| get_x_odrive_telem    | Driver        | prints position, velocity, etc.                                                         |
| x_hard_stop_home      | Driver        | moves odrive to detect home position                                                    |

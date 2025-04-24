## Router Commands

x = lox/ipa

## Operator Commands

| Command          | Module        | Function                                                       |
| ---------------- | ------------- | -------------------------------------------------------------- |
| ping             | Router        | prints pong (connection check)                                 |
| help             | Router        | prints all commands                                            |
| load_curve_sd    | Loader        | loads a curve from the sd card                                 |
| x_hard_stop_home | Driver        | moves odrive to detect home position                           |
| zero_pt_to_atm   | PT            | Sets **all** PT offsets to read 1 atm (14.7 psi)               |
| save_pt_zero     | PT (Loader)   | Save the current PT offsets to a file                          |
| restore_pt_zero  | PT (Loader)   | Load PT offsets from the most recent save                      |
| arm              | CurveFollower | performs safety checks, waits for zucrow, then follows a curve |
| print_sensors    | CurveFollower | prints readings from all connected sensors                     |

## Additional Debug Commands

| Command               | Module          | Function                                                          |
| --------------------- | --------------- | ----------------------------------------------------------------- |
| ls                    | SDCard          | list files                                                        |
| rm                    | SDCard          | remove a file                                                     |
| cat                   | SDCard          | prints file contents                                              |
| auto_cat              | SDCard          | prints file contents line by line, called by pull_file.py         |
| load_curve_serial     | Loader          | loads a curve over serial, called by send_curve.py                |
| write_curve_sd        | Loader          | saves the currently loaded curve to a file                        |
| spi_select            | SPI_Demux       | Toggles a CS line, used to debug sensor connections               |
| spi_deselect          | SPI_Demux       | Used to debug sensor connections                                  |
| zi_send_fault         | ZucrowInterface | Sets the fault (teensy -> zucrow) line to FAULT                   |
| zi_send_ok            | ZucrowInterface | Sets the fault (teensy -> zucrow) line to OK                      |
| zi_send_run           | ZucrowInterface | Sets the sync (teensy -> zucrow) line to RUN                      |
| zi_send_idle          | ZucrowInterface | Sets the sync (teensy -> zucrow) line to IDLE                     |
| zi_status_print       | ZucrowInterface | Prints the incoming state of the (zucrow -> teensy) control lines |
| kill                  | Driver          | moves odrives out of closed loop mode                             |
| get_odrive_info       | Driver          | prints odrive info to confirm connection                          |
| clear_x_odrive_errors | Driver          | resets odrive error state                                         |
| set_x_odrive_pos      | Driver          | manually move odrive                                              |
| get_x_cmd_pos         | Driver          | get last command pos sent to odrive                               |
| identify_x_odrive     | Driver          | blinks light on corresponding odrive                              |
| get_x_odrive_telem    | Driver          | prints position, velocity, etc.                                   |

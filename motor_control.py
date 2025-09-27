from pymodbus.client import ModbusSerialClient
from serial import Serial
import daq_serial_lib as dsl 
from time import sleep
import serial


# def control_script(motor: ModbusSerialClient):
# 	set_speed(motor, 200) # effects all commands()
# 	set_step_size(motor, 125) # effects step_forward and step_backward (currently step size is 1/8 of rotation)

# 	print("Forward")
# 	forward(motor)
# 	sleep(1)

# 	print("Reverse")
# 	reverse(motor)
# 	sleep(1)

# 	for _ in range(8):
# 		step_forward(motor)
# 		sleep(0.1)

# 	sleep(1)

# 	for _ in range(8):
# 		step_backward(motor)
# 		sleep(0.1)

# 	print("Stop")
# 	stop(motor)

def control(teensy: Serial, motor: ModbusSerialClient):
	dsl.arm_logging(teensy)
	dsl.start_logging(teensy)

	sleep(5)

	dsl.set_speed(motor, 50) # effects all commands()
	dsl.set_step_size(motor, 4500) # effects step_forward and step_backward (currently step size is 1/8 of rotation)

	dsl.step_forward(motor) # this is reverse
	sleep(10)
	dsl.step_backward(motor) # this is forward
	sleep(10)

	print("Stop")
	dsl.stop(motor)     

	while True:
		print(teensy.readline())
		if teensy.in_waiting:
			continue
		sleep(0.1)
		if not teensy.in_waiting:
			break


def main():
	try:
		motor = ModbusSerialClient(
			port=dsl.get_motor_port(),
			baudrate=115200,
			stopbits=1,
			bytesize=8,
			parity='N',
			timeout=1
		)

		# ---- CONNECT ----
		if not motor.connect():
			print("Failed to connect to the driver.")
			exit(1)

		teensy_port = dsl.get_teensy_port()

		teensy = serial.Serial(teensy_port, timeout=1)
		teensy.write(b"\n")
		teensy.read(256)
		teensy.write(b"ping\n")
		assert teensy.readline() == b'pong\r\n'
		print("Teensy connected ok!")

		control(teensy, motor)


	finally:
		print("Cleaning up!")
		motor.close()
		# teensy.close()

if __name__ == '__main__':
	main()
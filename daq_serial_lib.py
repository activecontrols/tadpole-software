from pymodbus.client import ModbusSerialClient
import serial.tools.list_ports
from serial import Serial
from time import sleep

def get_motor_port():
	for port, desc, hwid in sorted(serial.tools.list_ports.comports()):
			if "Silicon Labs CP210x USB to UART Bridge" in desc:
				print(f"Found motor port: {port}")
				return port
			
	print("Couldn't find the motor port. Make sure the device is connected and drivers are installed.")
	return input("Enter port name (like 'COM5'): ")

def get_teensy_port():
	for port, desc, hwid in sorted(serial.tools.list_ports.comports()):
			if "USB Serial Device" in desc:
				print(f"Found teensy port: {port}")
				return port
			
	print("Couldn't find the teensy port. Make sure the device is connected and drivers are installed.")
	return input("Enter port name (like 'COM5'): ")

def arm_logging(teensy: Serial):
	teensy.write(b"arm\n")
	sleep(0.1)
	teensy.write(f"{input("Log file name: ")}\n".encode())
	sleep(0.1)
	teensy.write(f"{input("Time: ")}\n".encode())

def start_logging(teensy: Serial):
	teensy.write(b"y\n")

def stop(motor: ModbusSerialClient):
	motor.write_register(0x004c, 0)

def forward(motor: ModbusSerialClient):
	motor.write_register(0x004c, 0)
	motor.write_register(0x004c, 3)
	motor.write_register(0x004c, 3)

def reverse(motor: ModbusSerialClient):
	motor.write_register(0x004c, 0)
	motor.write_register(0x004c, 4)
	motor.write_register(0x004c, 4)

def step_forward(motor: ModbusSerialClient):
	motor.write_register(0x004c, 0)
	motor.write_register(0x004c, 1)
	motor.write_register(0x004c, 1)

def step_backward(motor: ModbusSerialClient):
	motor.write_register(0x004c, 0)
	motor.write_register(0x004c, 2)
	motor.write_register(0x004c, 2)

def set_speed(motor: ModbusSerialClient, speed):
	motor.write_register(0x0043, speed)

def set_step_size(motor: ModbusSerialClient, pulses):
	motor.write_register(0x0046, pulses)
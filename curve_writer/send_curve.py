from time import sleep
import serial

port = 'COM5'

fname = input("Enter a filename and press enter to send curve: ")
device = serial.Serial(port, 115200, timeout=1)

device.write("ping\nload_curve_serial\n".encode())
with open("curve_writer/curves/" + fname, "rb") as f:
    device.write(f.read())

# print output  
while True:
	line = device.readline().decode()
	if (len(line) == 0):
		break
	print(line.strip())

from time import sleep
import serial

port = 'COM10'

fname = input("Enter a filename and press enter to activate the serial dump: ")
device = serial.Serial(port, 115200, timeout=1)

device.write("cat\n".encode())
sleep(1)
device.readline() #ignore the prompt
device.write(f"{fname}\n".encode())

with open("curve_writer/data/" + fname, "w+") as f:
    while True:
        lines = device.readlines()
        if (len(lines) == 0):
            break
        for line in lines:
            line = line.decode()
			# print(line.strip())
            f.write(line.strip() + "\n")

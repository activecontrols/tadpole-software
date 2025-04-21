from time import sleep
import serial

port = 'COM10'

fname = input("Enter a filename and press enter to activate the serial dump: ")
device = serial.Serial(port, 115200, timeout=1)

device.write("auto_cat\n".encode())
sleep(1)
device.readline() #ignore the prompt
device.write(f"{fname}\n".encode())

with open("curve_writer/data/" + fname, "w+") as f:
    while True:
        line = device.readline().decode()
        if (line.strip() == 'done'):
            break
        print(line.strip())
        f.write(line.strip() + "\n")
        device.write("\n".encode())

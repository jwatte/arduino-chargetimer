import serial
import time
s = serial.Serial("COM3", 9600, timeout=3)
time.sleep(2)
s.write("QA\r")
l = s.readline()
print repr(l)
s.write("SP,1\r")
time.sleep(0.2)
l = s.readline()
print repr(l)

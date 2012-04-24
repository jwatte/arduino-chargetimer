import serial
import time
import sys
import math


def checkCmd():
    global s
    l = s.readline()
    if (l != 'OK\r\n'):
        print "error: " + l + "\n"
        sys.exit(1)

def penDown():
    global s
    s.write("SP,0\r")
    time.sleep(0.5)
    checkCmd()

def penUp():
    global s
    s.write("SP,1\r")
    time.sleep(0.5)
    checkCmd()

def lineTo(x, y):
    s.write("SM,100,%d,%d\r" % (x, y))
    checkCmd()

s = serial.Serial("COM3", 9600, timeout=3)
time.sleep(2)
penUp()
lineTo(0, 0)
time.sleep(2)

penUp()
lineTo(100, 0)
penDown()
for i in range(0, 360):
    j = i * 3.1415927 / 180
    lineTo(int(100 * math.cos(j)), int(167 * math.sin(j)))
lineTo(100, 0)
penUp()
lineTo(0, 0)
time.sleep(2)


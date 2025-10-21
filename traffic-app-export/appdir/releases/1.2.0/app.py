#!/usr/bin/env python3
import RPi.GPIO as GPIO, time, os
RED=int(os.getenv("PIN_RED","17")); YEL=int(os.getenv("PIN_YEL","27")); GRN=int(os.getenv("PIN_GRN","22"))
ON=1; OFF=0
GPIO.setmode(GPIO.BCM); GPIO.setwarnings(False)
GPIO.setup([RED,YEL,GRN], GPIO.OUT, initial=GPIO.LOW)
GPIO.output(RED, OFF); GPIO.output(YEL, OFF); GPIO.output(GRN, ON)
try:
    while True: time.sleep(1)
finally: GPIO.cleanup()

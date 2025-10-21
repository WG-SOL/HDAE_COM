#!/usr/bin/env python3
import RPi.GPIO as GPIO, time, os

# BCM 핀: RED=17, YEL=27, GRN=22
RED=int(os.getenv("PIN_RED","17")); YEL=int(os.getenv("PIN_YEL","27")); GRN=int(os.getenv("PIN_GRN","22"))

# 공통음극 기준: HIGH=켜짐
ON=1; OFF=0  # 공통양극이면 ON=0; OFF=1 로 바꿔주세요

GPIO.setmode(GPIO.BCM); GPIO.setwarnings(False)
GPIO.setup([RED,YEL,GRN], GPIO.OUT, initial=GPIO.LOW)

GPIO.output(RED, ON)
GPIO.output(YEL, OFF)
GPIO.output(GRN, OFF)

try:
    while True:
        time.sleep(1)
finally:
    GPIO.cleanup()

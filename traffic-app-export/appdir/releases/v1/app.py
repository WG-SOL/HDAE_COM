#!/usr/bin/env python3
import RPi.GPIO as GPIO
import time, os, pathlib

# BCM 핀(공통음극: HIGH=켜짐)
RED = int(os.getenv("PIN_RED", "17"))
YEL = int(os.getenv("PIN_YEL", "27"))
GRN = int(os.getenv("PIN_GRN", "22"))

HEALTH_FILE = pathlib.Path("/opt/traffic-app/state/health_ok")

def on(p):  GPIO.output(p, 1)
def off(p): GPIO.output(p, 0)

def setup():
    GPIO.setmode(GPIO.BCM)
    GPIO.setwarnings(False)
    GPIO.setup([RED, YEL, GRN], GPIO.OUT, initial=GPIO.LOW)
    off(GRN)                 # 데모는 RED/YEL만 사용
    HEALTH_FILE.touch()      # 기동 성공 신호

def loop():
    last = time.time()
    while True:
        on(RED);  off(YEL); time.sleep(0.5)
        off(RED); on(YEL);  time.sleep(0.5)
        if time.time() - last > 5:
            HEALTH_FILE.touch(); last = time.time()

if __name__ == "__main__":
    try:
        setup(); loop()
    except KeyboardInterrupt:
        pass
    finally:
        GPIO.cleanup()

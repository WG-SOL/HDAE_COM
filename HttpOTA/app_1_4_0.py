#!/usr/bin/env python3
# v1.4.0 "삐까뻔쩍 왔다갔다" 패턴
import RPi.GPIO as GPIO
import time, os

# BCM 핀 (기본: RED=17, YEL=27, GRN=22)
RED = int(os.getenv("PIN_RED", "17"))
YEL = int(os.getenv("PIN_YEL", "27"))
GRN = int(os.getenv("PIN_GRN", "22"))

# 공통양극(LOW=켜짐) 쓰면 COMMON_ANODE=1 환경변수로 뒤집기
COMMON_ANODE = os.getenv("COMMON_ANODE", "0") == "1"
ON, OFF = (0, 1) if COMMON_ANODE else (1, 0)

GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(False)
GPIO.setup([RED, YEL, GRN], GPIO.OUT, initial=GPIO.LOW)

def set_rgb(r, y, g):
    GPIO.output(RED, ON if r else OFF)
    GPIO.output(YEL, ON if y else OFF)
    GPIO.output(GRN, ON if g else OFF)

def flash(pin_a, pin_b, times=12, interval=0.07):
    # 좌우 번쩍 (A<->B 교대)
    for _ in range(times):
        GPIO.output(pin_a, ON); GPIO.output(pin_b, OFF); time.sleep(interval)
        GPIO.output(pin_a, OFF); GPIO.output(pin_b, ON); time.sleep(interval)
    GPIO.output(pin_a, OFF); GPIO.output(pin_b, OFF)

def strobe(pin, times=10, on_ms=0.05, off_ms=0.05):
    # 빠른 스트로브 점멸
    for _ in range(times):
        GPIO.output(pin, ON);  time.sleep(on_ms)
        GPIO.output(pin, OFF); time.sleep(off_ms)

def sweep(seq, hold=0.12):
    # 순차 스윕 (예: R -> Y -> G -> Y)
    for pin in seq:
        GPIO.output(RED, OFF); GPIO.output(YEL, OFF); GPIO.output(GRN, OFF)
        GPIO.output(pin, ON)
        time.sleep(hold)
    GPIO.output(RED, OFF); GPIO.output(YEL, OFF); GPIO.output(GRN, OFF)

def breathe(pin, cycles=1):
    # 소프트 '브레스' (PWM 없이 의사 브라이트)
    for _ in range(cycles):
        for t in [0.02,0.03,0.04,0.05,0.06,0.07,0.08]:
            GPIO.output(pin, ON); time.sleep(t)
            GPIO.output(pin, OFF); time.sleep(max(0.01, 0.09 - t))

try:
    while True:
        # 1) 빨강↔노랑 삐까뻔쩍
        flash(RED, YEL)

        # 2) 초록 스트로브
        strobe(GRN)

        # 3) 스윕: R → Y → G → Y
        sweep([RED, YEL, GRN, YEL])

        # 4) 트리플 러너
        for pin in [RED, YEL, GRN, YEL]:
            GPIO.output(pin, ON); time.sleep(0.06); GPIO.output(pin, OFF)

        # 5) 빨강 브레스
        breathe(RED, 1)

        # 6) 하이라이트
        set_rgb(1,1,1); time.sleep(0.12)
        set_rgb(0,0,0); time.sleep(0.10)

except KeyboardInterrupt:
    pass
finally:
    GPIO.output(RED, OFF); GPIO.output(YEL, OFF); GPIO.output(GRN, OFF)
    GPIO.cleanup()

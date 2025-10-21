#!/usr/bin/env python3
# v1.3.0  랜덤 롱온 + 랜덤 타임슬라이스 쇼
import RPi.GPIO as GPIO
import time, os, random

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

PINS = [RED, YEL, GRN]

def all_off():
    GPIO.output(RED, OFF); GPIO.output(YEL, OFF); GPIO.output(GRN, OFF)

def only(pin):
    for p in PINS:
        GPIO.output(p, ON if p == pin else OFF)

def random_long_on():
    """하나를 랜덤으로 골라 '길게' 켜두기 (길이도 랜덤)"""
    pin = random.choice(PINS)
    dur = random.uniform(0.8, 2.2)  # 0.8~2.2초
    only(pin)
    time.sleep(dur)
    return pin, dur

def random_flash_show(total_time=None):
    """랜덤 타임슬라이스로 현란하게 왔다갔다"""
    # 총 쇼 길이도 랜덤 (지정 없으면 1.8~3.2초)
    t_end = time.time() + (total_time if total_time is not None else random.uniform(1.8, 3.2))
    last_pin = None
    while time.time() < t_end:
        # 다음 켤 핀 (이전과 다를 확률 높이기)
        candidates = [p for p in PINS if p != last_pin] or PINS
        pin = random.choice(candidates)
        # 랜덤 듀티 (40~150ms)
        dwell = random.uniform(0.04, 0.15)
        # 10% 확률로 스파크(두 개 동시에 아주 잠깐)
        if random.random() < 0.10:
            others = [p for p in PINS if p != pin]
            spark = random.choice(others)
            GPIO.output(pin, ON); GPIO.output(spark, ON)
            time.sleep(min(0.06, dwell))
            GPIO.output(spark, OFF)
            # 남은 시간만큼 pin 유지
            rest = max(0, dwell - 0.06)
            if rest > 0: time.sleep(rest)
        else:
            only(pin); time.sleep(dwell)
        last_pin = pin

    all_off()
    # 쇼가 끝날 때 살짝 하이라이트(짧게 전체 점등) 줄 수도 있음
    if random.random() < 0.35:
        GPIO.output(RED, ON); GPIO.output(YEL, ON); GPIO.output(GRN, ON)
        time.sleep(0.08 + random.uniform(0, 0.06))
    all_off()

try:
    all_off()
    while True:
        # 1) 랜덤 롱온
        random_long_on()
        # 2) 랜덤 타임슬라이스 쇼
        random_flash_show()
        # 3) 루프 사이 간격도 살짝 랜덤
        time.sleep(random.uniform(0.05, 0.2))

except KeyboardInterrupt:
    pass
finally:
    all_off()
    GPIO.cleanup()

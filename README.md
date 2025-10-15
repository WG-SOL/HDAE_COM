# 현대오토에버 모빌리티 SW스쿨 2기_Embedded

## TEAM 1

## Team
<table>
  <tr>
    <td align="center">1</td>
    <td align="center">2</td>
    <td align="center">3</td>
    <td align="center">4</td>
    <td align="center">5</td>
    <td align="center">6</td>
  </tr>
     <tr>
    <td align="center"><a href="https://github.com/"><sub><b>기동언</b></td>
    <td align="center"><a href="https://github.com/"><sub><b>유원규</b></td>
    <td align="center"><a href="https://github.com/"><sub><b>심동현</b></td>
    <td align="center"><a href="https://github.com/"><sub><b>이나연</b></td>
    <td align="center"><a href="https://github.com/"><sub><b>김종훈</b></td>  
  </tr>
      
</table>

<br><br>

## 🛠 Stack

### 💻 Languages
![C](https://img.shields.io/badge/C-%2300599C.svg?style=for-the-badge&logo=c&logoColor=white)
![C++](https://img.shields.io/badge/C++-%2300599C.svg?style=for-the-badge&logo=c%2B%2B&logoColor=white)

### 🔧 Embedded System
![Embedded](https://img.shields.io/badge/Embedded-%231572B6.svg?style=for-the-badge&logo=platformdotio&logoColor=white)
![Infineon](https://img.shields.io/badge/Infineon-A8B400.svg?style=for-the-badge&logo=infineon&logoColor=white)

### ⚙️ Tools
![Git](https://img.shields.io/badge/Git-F05032.svg?&style=for-the-badge&logo=Git&logoColor=white)
![AURIX Studio](https://img.shields.io/badge/AURIX%20Studio-0088CC.svg?style=for-the-badge)
![UDE Platform](https://img.shields.io/badge/UDEPlatform-D2232A.svg?style=for-the-badge)


## ✔️ 주요 기능

> **씽씽이**

- **달려라**


<br>




## &#128215; 아키텍처

그림 그려라
    
<br>   <br>

📙 포팅메뉴얼
1) 공통
sudo apt-get update
sudo apt-get install -y build-essential cmake git gdb pkg-config

2) Qt
sudo apt-get install -y qtcreator qtbase5-dev qt5-qmake \
  qt5-doc qt5-doc-html qtbase5-doc-html qtbase5-examples
qtcreator


(WSL 크롬 미리보기)

cd /tmp
wget https://dl.google.com/linux/direct/google-chrome-stable_current_amd64.deb
sudo apt install -f ./google-chrome-stable_current_amd64.deb

3) MQTT

Windows(브로커)

C:\Program Files\mosquitto\mosquitto.conf
listener 1883
allow_anonymous true

cd "C:\Program Files\mosquitto"
mosquitto install
net start mosquitto


방화벽: ICMPv4 Echo 허용, TCP 1883 인바운드 허용

WSL/RPi(클라이언트/브로커)

sudo apt-get install -y mosquitto mosquitto-clients
sudo systemctl enable mosquitto && sudo systemctl start mosquitto

# 테스트 (PC 브로커 IP 예: 10.174.91.164)
mosquitto_sub -h 10.174.91.164 -t "test/topic"
mosquitto_pub -h 10.174.91.164 -t "test/topic" -m "Hello"


(C++ 예)

sudo apt-get install -y g++ libmosquittopp-dev
g++ publisher.cpp -o publisher -lmosquittopp
./publisher

4) vSomeIP
sudo apt-get install -y libboost-all-dev libsystemd-dev libdlt-dev
git clone https://github.com/COVESA/vsomeip.git
cd vsomeip && mkdir build && cd build
cmake .. && make -j$(nproc)
sudo make install && sudo ldconfig


런타임 환경변수:

export VSOMEIP_CONFIGURATION=/home/won/vsomeip/config/tc375_client.json
export VSOMEIP_APPLICATION_NAME=client-sample

5) CAN-TP (MCP2515)
sudo sudoedit /boot/firmware/config.txt
# 아래 두 줄 추가 (오실레이터는 모듈 표기 확인)
dtparam=spi=on
dtoverlay=mcp2515-can0,oscillator=12000000,interrupt=25
sudo reboot

sudo ip link set can0 up type can bitrate 500000
ip -details link show can0
sudo apt-get install -y can-utils

# vcan 테스트
sudo modprobe vcan
sudo ip link add dev vcan0 type vcan
sudo ip link set up vcan0

# ISOTP
isotprecv -s 0x7E8 -d 0x7E0 -l vcan0
echo "11 22 33 44 55 66 77 88 99 AA BB CC DD EE FF 00" \
 | isotpsend -s 0x7E0 -d 0x7E8 vcan0

6) Bluetooth 리시버
sudo apt-get install -y libbluetooth-dev
bluetoothctl <<EOF
power on
agent on
default-agent
scan on
# MAC 확인 후
# pair XX:XX:XX:XX:XX:XX ; trust ... ; connect ...
EOF

7) 네트워크 프리셋(수동, 부팅 시 반복)
# 유선 고정 IP 예시
sudo ip addr add <IPv4/24> dev eth0
sudo ip link set eth0 up
ip route
ip addr show eth0
ping <상대 IPv4> -c 4


Troubleshooting 핵심

vSomeIP: Boost ≥1.66, JSON 경로/앱이름 환경변수 필수

CAN-TP: oscillator 값 실제 크리스털과 일치, 120Ω 종단 확인

MQTT: 방화벽(ICMP, TCP 1883) 및 브로커 IP 확인

BLE: pair → trust → connect 순서와 권한


## &#128187; 서비스 화면

## 💡 브랜치 전략

Github flow + 담당repo + dev

- master : 무결성 유지, dev 브랜치에서만 PR 가능
- dev : 개발용, 기능 브랜치들 merge용, 버그 해결용
- 기능 : 새로운 기능 개발용 (브랜치명을 명시적으로 작성)

```
master
 └─ dev
     ├── Back-End
     |     ├── 브랜치 1
     |     └── 브랜치 2
     ├── Front-End
     |     ├── 브랜치 1
     |     └── 브랜치 2
     └── Embedded
           ├── 브랜치 1
           └── 브랜치 2
```

<br><br>

## 🤙🏻 commit 컨벤션

```
💡 feat : 새로운 기능 추가
🐞 fix : 버그 수정
📄 docs : 문서 수정
🛠 refact : 코드 리팩토링
💅 style : 코드 의미에 영향을 주지 않는 변경사항
📦 chore : 빌드 부분 혹은 패키지 매니저 수정사항
```

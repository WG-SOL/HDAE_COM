# HDAE_COM
현대오토에버 통신 프로젝트

// RaspberryPi Pre-Setting
// 정적으로 설정하는거라 켤 때마다 해줘야 됨
// 설정으로 빼서 자동화 가능한데 나는 그러면 wlan0에 문제 생겨서 수동으로 함

sudo ip addr add IpV4/24 dev eth0
sudo ip link set eth0 up
ip route
ip addr show eth0
ping IpV4

//eth0 Interface inet 확인
ifconfig -a

// CONFIG만 개인 디렉토리 계층구조에 맞게 변경해주고 가서 실행하면 됩니다.
export VSOMEIP_CONFIGURATION=/home/won/vsomeip/config/tc375_client.json
export VSOMEIP_APPLICATION_NAME=client-sample


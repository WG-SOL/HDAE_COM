sudo raspi-config
sudo reboot
cd RaspberryPi/
cd main2
./main
ls
./main
g++ main.cpp can.cpp color.cpp draw.cpp lane_detect.cpp -o main     `pkg-config --cflags --libs opencv4`     -std=c++17
./main
g++ main.cpp can.cpp color.cpp draw.cpp lane_detect.cpp -o main     `pkg-config --cflags --libs opencv4`     -std=c++17
./main
sudo ip link set can0 down
ifconfig
sudo ip link set can0 type can bitrate 500000~
sudo ip link set can0 type can bitrate 500000
sudo nano /boot/config.txt
sudo nano /boot/firmware/config.txt
ip addr
sudo ip link set can0 up type can bitrate 500000
sudo reboot
sudo RaspberryPi/
cd RaspberryPi/
cd main2
ip addr
sudo ip link set can0 up type can bitrate 500000
./main
[200~nano start_lkas.sh~
nano start_lkas.sh~
nano start_lkas.sh
sudo reboot
cd RaspberryPi/
cd main2
chmod +x start_lkas.sh
nano ~/.config/lxsession/LXDE-pi/autostart
mkdir -p ~/.config/autostar
nano ~/.config/autostart/lkas.desktop
nano ~/.config/lxsession/LXDE-pi/autostart
mkdir -p ~/.config/lxsession/LXDE-pi
nano ~/.config/lxsession/LXDE-pi/autostart
sudo reboot
ls
cd over-the-air/
ls
cd ab_update/
ls
cd ../..
ls
mkdir PJ2
ls
rm -r PJ2/
ls
mkdir Pj2
ls
cd P
cd Pj2/
ls
cd P
cd Pj2/
ls
cd ..
ifconfig
mosquitto_sub -h 10.174.91.164 -t "test/topic"
mosquitto_sub -h localhost -t "test/topic"
mosquitto_sub -h 10.174.91.164 -t "test/topic"
ls
sudo apt update
sudo apt install build-essential g++ libmosquittopp-dev
lls
ls
cd Pj2
ls
vi subscriber.cpp
g++ subscriber.cpp -o subscriber -lmosquittopp
./subscriber
ls
cd Pj2/
ls
./subscriber 
ls
cd Pj2/
ls
./subscriber
vi subscriber
vi subscriber.cpp
./subscriber
ls
sudo apt-get update
sudo apt-get install can-utils -y
isotprecv -s 0x7E8 -d 0x7E0 -l can0
isotprecv -s 0x7E8 -d 0x7E0 -l vcan0
isotpsend -s 0x7E0 -d 0x7E8 can0 112233445566778899AABBCCDDEEFF00
echo "112233445566778899AABBCCDDEEFF00" | isotpsend -s 0x7E0 -d 0x7E8 can0
sudo modprobe vcan
sudo ip link add dev vcan0 type vcan
sudo ip link set up vcan0
echo "112233445566778899AABBCCDDEEFF00" | isotpsend -s 0x7E8 -d 0x7E0 vcan0
echo "112233445566778899AABBCCDDEEFF00" | isotpsend -s 0x7E0 -d 0x7E8 can0
echo "112233445566778899AABBCCDDEEFF00" | isotpsend -s 0x7E8 -d 0x7E0 vcan0
echo "112233445566778899AABBCCDDEEFF00" | isotpsend -s 0x7E0 -d 0x7E8 vcan0
echo "112233445566778899AABBCCDDEEFF00" | isotpsend -s 0x7E8 -d 0x7E0 vcan0
echo "112233445566778899AABBCCDDEEFF00" | isotpsend -s 0x7E0 -d 0x7E8 vcan0
echo "11 22 33 44 55 66 77 88 99 AA BB CC DD EE FF 00" | isotpsend -s 0x7E0 -d 0x7E8 vcan0
sudo vi /boot/config.txt
cd /
ls
vi boot
cd boot
ls
vi config.txt
ls
cd firmware/
ls
vi config.txt
ls
sudo chmod +x 777 config.txt
sudo chmod +x config.txt
ls
vi config.txt
sudo chmod 777 config.txt
vi config.txt
sudo vi config.txt
sudo ip link set can0 up type can bitrate 500000
cd ~
ls
cd Pj2/
ls
vi isotp_example.cpp
g++ -o isotp_test isotp_example.cpp
./isotp_test
vi isotp_test
vi isotp_test.c
ls
vi isotp_example.cpp
ls
cd Pj2
l
ls
vi can_send.cpp
g++ -o can_send_test can_send.cpp
./can_send
./can_send_test 
ls
vi can_send.cpp
./can_send_test 
vi isotp_test
vi isotp_example.cpp
ls
cd over-the-air/
ls
cd ..
cd Pj2
ls
ls
cd P
cd Pj2/
ls
./can_send_test
./isotp_test
sudo vi /boot/firmware/config.txt
sudo ip link set can0 up type can bitrate 500000
sudo apt-get install can-utils -y
sudo ip link set can0 up type can bitrate 500000
sudo vi /boot/firmware/config.txt
sudo reboot
ls
cd Pj2/
ls
./can_send_test 
./isotp_test
vi can_send.cpp 
ifconfig can0
./isotp_test
ls
isotprecv -s 0x7E8 -d 0x7E0 -l vcan0
sudo modprobe vcan
sudo ip link add dev vcan0 type vcan
sudo ip link set up vcan0
isotprecv -s 0x7E8 -d 0x7E0 -l vcan0
sudo ip link set can0 up type can bitrate 500000
./isotp_test
./can_send_test 
ls
vi can_send.cpp 
sudo ip link set can0 up type can bitrate 500000
./can_send_test 
vi can_send.cpp 
./can_send_test 
vi can_send_test
vi can_send.cpp 
ip -details link show can0
sudo reboot
echo "11 22 33 44 55 66 77 88 99 AA BB CC DD EE FF 00" | isotpsend -s 0x7E0 -d 0x7E8 vcan0
cd Pj2/
ls
ip -details link show can0
sudo ip link set can0 up type can bitrate 500000
ip -details link show can0
./can_send_test 
vi can_send.cpp 
g++ -o can_send_test can_send.cpp
vi can_send.cpp 
g++ -o can_send_test can_send.cpp
./can_send_test 
ip -details link show can0
sudo ip link set can0 up type can bitrate 500000
ip -details link show can0
sudo reboot
ls
cd Pj2/
ls
vi can_send.cpp
ip -details link show can0
./can_send_test
g++ -o can_send_test can_send.cpp
ip -details link show can0
./can_send_test
sudo ip link set can0 up type can bitrate 500000
./can_send_test
candump can0
sudo ip link set can0 up type can bitrate 500000
candump can0
sudo ip link set can0 down
sudo ip link set can0 type can loopback on
sudo ip link set can0 up
candump can0
cansend can0 123#112233
ls
cd Pj2/
ls
vi can_send.cpp
ipconfig can
ifconfig can
ifconfig can0
candump can0
sudo ip link set can0 up type can bitrate 500000
candump can0
ifconfig can0
ip -details link show can0
sudo vi /boot/firmware/config.txt
sudo reboot
ifsudo ip link set can0 up type can bitrate 500000
sudo ip link set can0 up type can bitrate 500000
ip -details link show can0
cd /boot
ls
vi config.txt
ls
cd firmware/
sl
ls
vi config.txt
cat config.txt
vi config.txt
sudo vi config.txt
sudo apt update
sudo apt full-upgrade -y
sudo reboot
sudo vi /boot/firmware/config.txt 
sudo ip link set can0 up type can bitrate 500000
sudo reboot
sudo vi /boot/firmware/config.txt 
sudo reboot
ls
sudo ip link set can0 up type can bitrate 500000
ip -details link show can0
sudo vi /boot/firmware/config.txt 
sudo reboot
sudo ip link set can0 up type can bitrate 500000
ip -details link show can0
can0 dump
candump can0
cd P
cd Pj2/
ls
./can_send_test
./isotp_test
./can_send_test
./isotp_test
ls
cd Pj2/
ls
g++ -o isotp_client isotp_client.cpp
g++ -o isotp_client isotp_example.cpp
sudo ip link set can0 up type can bitrate 500000
./isotp_client
./isotp_client 
ls
vi receive.cpp
vi send_long.cpp
vi receive.cpp
g++ -o receive receive.cpp
g++ -o send send_long.cpp
./send
./receive 
./send
./receive 

#!/bin/bash

# Wait for 10 seconds after booting (time can be adjusted)
echo "======= Boot complete. Starting the program in 10 seconds... ======="
sleep 10

# Change to the script's directory (Important!)
cd "$(dirname "$0")"

echo "======= 1. Starting CAN interface setup ======="
sudo ip link set can0 down
sudo ip link set can0 type can bitrate 500000
sudo ip link set can0 up
echo "======= CAN setup complete. ======="
sleep 1

echo ""
echo "======= 2. Starting the program... ======="
./main

echo ""
echo "======= Program has finished. ======="
# Wait to prevent the terminal from closing immediately
read -p "Press Enter to close the terminal..."

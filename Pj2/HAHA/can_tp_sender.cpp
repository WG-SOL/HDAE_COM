#include "can_tp_sender.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/isotp.h>

CanTpSender::CanTpSender(const std::string& interfaceName) : socket_fd(-1), is_initialized(false) {
    is_initialized = init(interfaceName);
}

CanTpSender::~CanTpSender() {
    if (socket_fd >= 0) {
        close(socket_fd);
        std::cout << "[CAN] 소켓을 닫았습니다." << std::endl;
    }
}

bool CanTpSender::init(const std::string& interfaceName) {
    // 1. ISO-TP 소켓 생성
    socket_fd = socket(PF_CAN, SOCK_DGRAM, CAN_ISOTP);
    if (socket_fd < 0) {
        perror("[CAN] Socket 생성 실패");
        return false;
    }

    // 2. CAN 인터페이스 지정
    struct ifreq ifr;
    strncpy(ifr.ifr_name, interfaceName.c_str(), IFNAMSIZ - 1);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';
    if (ioctl(socket_fd, SIOCGIFINDEX, &ifr) < 0) {
        perror(("[CAN] ioctl 실패: " + interfaceName + " 인터페이스를 찾을 수 없습니다").c_str());
        close(socket_fd);
        socket_fd = -1;
        return false;
    }

    // 3. CAN 주소 구조체 설정 (UDS 표준 ID 사용)
    struct sockaddr_can addr;
    memset(&addr, 0, sizeof(addr));
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    addr.can_addr.tp.rx_id = 0x7E8; // TC375(ECU)로부터 받을 응답 ID
    addr.can_addr.tp.tx_id = 0x7E0; // TC375(ECU)로 보낼 요청 ID

    // 4. 소켓에 주소 바인딩
    if (bind(socket_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("[CAN] Bind 실패");
        close(socket_fd);
        socket_fd = -1;
        return false;
    }

    std::cout << "[CAN] CAN-TP Sender가 '" << interfaceName << "' 인터페이스에서 성공적으로 초기화되었습니다." << std::endl;
    return true;
}

bool CanTpSender::sendMessage(const std::vector<unsigned char>& data) {
    if (!is_initialized || socket_fd < 0) {
        std::cerr << "[CAN] CAN Sender가 초기화되지 않았습니다." << std::endl;
        return false;
    }

    ssize_t bytes_sent = write(socket_fd, data.data(), data.size());
    if (bytes_sent < 0) {
        perror("[CAN] Write 실패");
        return false;
    }

    if (static_cast<size_t>(bytes_sent) != data.size()) {
        std::cerr << "[CAN] 데이터의 일부만 전송되었습니다." << std::endl;
        return false;
    }
    
    return true;
}

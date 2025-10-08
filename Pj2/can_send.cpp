#include <iostream>
#include <cstring>
#include <string>

// C++에서 C 스타일의 헤더를 사용하기 위한 헤더들
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>

int main() {
    // 1. 소켓 생성 (PF_CAN, RAW 소켓, CAN_RAW 프로토콜)
    int s; // 소켓 디스크립터
    if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
        perror("Socket 생성 실패");
        return 1;
    }

    // 2. 사용할 CAN 인터페이스 지정 (예: can0)
    struct ifreq ifr;
    strcpy(ifr.ifr_name, "can0");
    if (ioctl(s, SIOCGIFINDEX, &ifr) < 0) {
        perror("ioctl 실패: CAN 인터페이스를 찾을 수 없습니다");
        close(s);
        return 1;
    }

    // 3. 소켓을 CAN 인터페이스에 바인드
    struct sockaddr_can addr;
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Bind 실패");
        close(s);
        return 1;
    }

    // 4. 전송할 CAN 프레임(Frame) 정의
    struct can_frame frame;

    // CAN ID 설정 (Standard ID, 11-bit)
    frame.can_id = 0x123; 

    // 데이터 길이(DLC) 설정 (0~8 바이트)
    frame.can_dlc = 8;

    // 보낼 데이터 채우기
    frame.data[0] = 0x11;
    frame.data[1] = 0x22;
    frame.data[2] = 0x33;
    frame.data[3] = 0x44;
    frame.data[4] = 0x55;
    frame.data[5] = 0x66;
    frame.data[6] = 0x77;
    frame.data[7] = 0x88;

    // 5. CAN 프레임 전송
    ssize_t nbytes = write(s, &frame, sizeof(struct can_frame));

    if (nbytes < 0) {
        perror("Write 실패");
        close(s);
        return 1;
    }

    if (nbytes < sizeof(struct can_frame)) {
        std::cerr << "불완전한 프레임이 전송되었습니다." << std::endl;
        close(s);
        return 1;
    }

    std::cout << "CAN ID " << std::hex << frame.can_id << "으로 " 
              << static_cast<int>(frame.can_dlc) << " 바이트 전송 완료!" << std::endl;

    // 6. 소켓 닫기
    close(s);

    return 0;
}

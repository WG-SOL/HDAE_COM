#include <iostream>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/isotp.h>

int main() {
    int s; // 소켓 파일 디스크립터
    struct sockaddr_can addr;
    struct ifreq ifr;

    // 1. ISO-TP 소켓을 생성합니다. (SOCK_DGRAM)
    if ((s = socket(PF_CAN, SOCK_DGRAM, CAN_ISOTP)) < 0) {
        perror("Socket 생성 실패");
        return 1;
    }

    // 2. 사용할 CAN 인터페이스로 'can0'를 지정합니다.
    strcpy(ifr.ifr_name, "can0");
    if (ioctl(s, SIOCGIFINDEX, &ifr) < 0) {
        perror("ioctl 실패: CAN 인터페이스(can0)를 찾을 수 없습니다");
        close(s);
        return 1;
    }

    // 3. CAN 주소 구조체를 설정합니다.
    //    UDS(차량진단) 표준 ID를 사용합니다. 라즈베리파이가 '진단기' 역할을 합니다.
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    addr.can_addr.tp.rx_id = 0x7E8; // TC375(ECU)로부터 받을 응답 ID
    addr.can_addr.tp.tx_id = 0x7E0; // TC375(ECU)로 보낼 요청 ID

    // 4. 소켓에 주소 정보를 바인딩합니다.
    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Bind 실패");
        close(s);
        return 1;
    }

    // 5. TC375로 전송할 20바이트 길이의 데이터를 준비합니다.
    std::vector<unsigned char> data_to_send = {
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
        0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29
    };

    // 6. 데이터를 전송합니다.
    std::cout << "TC375로 20바이트 데이터 전송 시도... (CAN ID: 0x" << std::hex << addr.can_addr.tp.tx_id << ")" << std::endl;
    ssize_t bytes_sent = write(s, data_to_send.data(), data_to_send.size());

    if (bytes_sent < 0) {
        perror("Write 실패");
        close(s);
        return 1;
    }

    std::cout << "메시지 전송 완료. (" << bytes_sent << " 바이트 처리 요청됨)" << std::endl;

    // 7. 소켓을 닫고 프로그램을 종료합니다.
    close(s);
    return 0;
}


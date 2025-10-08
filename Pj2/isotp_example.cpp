#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/isotp.h>

int main() {
    int s;
    struct sockaddr_can addr;
    struct ifreq ifr;

    // 1. ISO-TP 소켓 생성
    if ((s = socket(PF_CAN, SOCK_DGRAM, CAN_ISOTP)) < 0) {
        perror("Socket 생성 실패");
        return 1;
    }

    // 2. can0 인터페이스 지정
    strcpy(ifr.ifr_name, "can0");
    if (ioctl(s, SIOCGIFINDEX, &ifr) < 0) {
        perror("ioctl 실패: CAN 인터페이스(can0)를 찾을 수 없습니다");
        close(s);
        return 1;
    }

    // 3. CAN 주소 구조체 설정 (UDS 표준 ID 사용)
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    addr.can_addr.tp.rx_id = 0x7E8; // TC375가 보낼 응답 ID
    addr.can_addr.tp.tx_id = 0x7E0; // TC375로 보낼 요청 ID

    // 4. 소켓에 주소 바인딩
    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Bind 실패");
        close(s);
        return 1;
    }

    // 5. 전송할 데이터 준비 (20 바이트)
    std::vector<unsigned char> data_to_send = {
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
        0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29
    };

    // 6. 데이터 전송
    std::cout << "TC375로 20바이트 데이터 전송 시도... (CAN ID: 0x" << std::hex << addr.can_addr.tp.tx_id << ")" << std::endl;
    ssize_t bytes_sent = write(s, data_to_send.data(), data_to_send.size());
    if (bytes_sent < 0) {
        perror("Write 실패");
        close(s);
        return 1;
    }
    std::cout << bytes_sent << " 바이트 데이터 전송 완료!" << std::endl;

    // 7. 응답 데이터 수신 (TC375가 응답할 때까지 대기)
    std::cout << "TC375의 응답 대기 중... (CAN ID: 0x" << std::hex << addr.can_addr.tp.rx_id << ")" << std::endl;
    unsigned char recv_buf[4096];
    ssize_t bytes_read = read(s, recv_buf, sizeof(recv_buf));

    if (bytes_read < 0) {
        perror("Read 실패");
        close(s);
        return 1;
    }

    // 8. 수신된 응답 데이터 출력
    std::cout << bytes_read << " 바이트 데이터 수신:" << std::endl;
    for (ssize_t i = 0; i < bytes_read; ++i) {
        // 문자로 출력 가능한 경우 문자로, 아니면 16진수로 출력
        if (isprint(recv_buf[i])) {
            printf("%c", recv_buf[i]);
        } else {
            printf(" 0x%02X ", recv_buf[i]);
        }
    }
    printf("\n");

    // 9. 소켓 닫기
    close(s);

    return 0;
}

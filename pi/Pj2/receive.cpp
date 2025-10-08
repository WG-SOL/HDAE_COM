#include <iostream>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/isotp.h>

// 수신된 데이터를 16진수 형식으로 예쁘게 출력하는 함수
void print_hex(const std::vector<unsigned char>& data) {
    for (size_t i = 0; i < data.size(); ++i) {
        printf("%02X ", data[i]);
    }
    printf("\n");
}

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
    //    TC375가 'ECU' 역할을 하므로, ECU가 보내는 응답 ID를 수신합니다.
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    addr.can_addr.tp.rx_id = 0x7E8; // TC375(ECU)가 보낼 ID (우리가 수신할 ID)
    addr.can_addr.tp.tx_id = 0x7E0; // TC375(ECU)로 보낼 ID (지금은 사용 안함)

    // 4. 소켓에 주소 정보를 바인딩합니다.
    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Bind 실패");
        close(s);
        return 1;
    }

    // 5. TC375로부터 메시지가 올 때까지 대기합니다.
    std::cout << "TC375로부터 메시지 수신 대기 중... (수신 ID: 0x" << std::hex << addr.can_addr.tp.rx_id << ")" << std::endl;
    
    std::vector<unsigned char> recv_buf(4096); // 최대 4095바이트까지 수신 가능한 버퍼
    ssize_t bytes_read = read(s, recv_buf.data(), recv_buf.size());

    if (bytes_read < 0) {
        perror("Read 실패");
        close(s);
        return 1;
    }

    // 6. 데이터가 수신되면, 실제 수신된 크기만큼 버퍼를 조절하고 내용을 출력합니다.
    if (bytes_read > 0) {
        recv_buf.resize(bytes_read);
        std::cout << ">> " << bytes_read << " 바이트 메시지 수신 완료:" << std::endl;
        print_hex(recv_buf);
    }

    // 7. 소켓을 닫고 프로그램을 종료합니다.
    close(s);
    return 0;
}


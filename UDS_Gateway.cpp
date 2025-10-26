#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <optional>
#include <stdexcept>
#include <mosquittopp.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <linux/can.h>
#include <linux/can/isotp.h>
#include <net/if.h>
#include <sys/ioctl.h>

// ============================================================
// 🧩 환경 설정
// ============================================================
const char* CAN_IFACE = "can0";
const uint16_t UDS_CAN_REQ = 0x7E0;
const uint16_t UDS_CAN_RESP = 0x7E8;

const char* ECU_IP = "192.168.2.30";
const int DOIP_PORT = 13400;

const char* MQTT_BROKER_IP = "10.152.189.222";
const int MQTT_PORT = 1883;
const char* MQTT_COMMAND_TOPIC = "request/topic";
const char* MQTT_RESPONSE_TOPIC = "response/topic";

// ============================================================
// 🧩 리눅스용 소켓 정의 (Windows 호환 제거)
// ============================================================
#ifndef _WIN32
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR   -1
#define closesocket(s) close(s)
#endif

// ============================================================
// 🧩 HEX 출력 도우미
// ============================================================
void print_hex(const std::string& prefix, const std::vector<uint8_t>& data) {
    std::cout << prefix;
    for (auto b : data)
        std::cout << std::hex << std::uppercase << std::setw(2)
                  << std::setfill('0') << (int)b << ' ';
    std::cout << std::dec << std::endl;
}

std::string bytes_to_hex_string(const std::vector<uint8_t>& bytes) {
    std::stringstream ss;
    ss << std::hex << std::uppercase << std::setfill('0');
    for (auto b : bytes)
        ss << std::setw(2) << (int)b;
    return ss.str();
}

// ============================================================
// 🧩 CAN-TP (ISO-TP) 전송 함수
// ============================================================
std::optional<std::vector<uint8_t>> sendCanTpRequest(const std::vector<uint8_t>& request) {
    int s;
    struct sockaddr_can addr;
    struct ifreq ifr;
    unsigned char recv_buf[4096];

    if ((s = socket(PF_CAN, SOCK_DGRAM, CAN_ISOTP)) < 0) {
        perror("[CAN] Socket 생성 실패");
        return std::nullopt;
    }

    strcpy(ifr.ifr_name, CAN_IFACE);
    if (ioctl(s, SIOCGIFINDEX, &ifr) < 0) {
        perror("[CAN] ioctl 실패");
        close(s);
        return std::nullopt;
    }

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    addr.can_addr.tp.rx_id = UDS_CAN_RESP;
    addr.can_addr.tp.tx_id = UDS_CAN_REQ;

    if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("[CAN] Bind 실패");
        close(s);
        return std::nullopt;
    }

    // 요청 전송
    ssize_t sent = write(s, request.data(), request.size());
    if (sent < 0) {
        perror("[CAN] Write 실패");
        close(s);
        return std::nullopt;
    }

    // 응답 수신
    ssize_t recv_len = read(s, recv_buf, sizeof(recv_buf));
    if (recv_len <= 0) {
        std::cerr << "[CAN] 응답 없음" << std::endl;
        close(s);
        return std::nullopt;
    }

    std::vector<uint8_t> response(recv_buf, recv_buf + recv_len);
    print_hex("[CAN] 응답: ", response);

    close(s);
    return response;
}

// ============================================================
// 🧩 DoIP (TCP) 전송 함수
// ============================================================
std::optional<std::vector<uint8_t>> sendDoipRequest(const std::vector<uint8_t>& uds) {
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        perror("[DoIP] 소켓 생성 실패");
        return std::nullopt;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(DOIP_PORT);
    inet_pton(AF_INET, ECU_IP, &addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        perror("[DoIP] 연결 실패");
        closesocket(sock);
        return std::nullopt;
    }

    // 요청 전송
    int sent = send(sock, (const char*)uds.data(), uds.size(), 0);
    if (sent <= 0) {
        perror("[DoIP] 송신 실패");
        closesocket(sock);
        return std::nullopt;
    }

    // 응답 수신
    std::vector<uint8_t> buffer(2048);
    int recv_len = recv(sock, (char*)buffer.data(), buffer.size(), 0);
    if (recv_len <= 0) {
        std::cerr << "[DoIP] 응답 없음" << std::endl;
        closesocket(sock);
        return std::nullopt;
    }

    buffer.resize(recv_len);
    print_hex("[DoIP] 응답: ", buffer);

    closesocket(sock);
    return buffer;
}

// ============================================================
// 🧩 MQTT Bridge 클래스
// ============================================================
class MqttBridge : public mosqpp::mosquittopp {
public:
    MqttBridge(const char* id) : mosqpp::mosquittopp(id) {}

    void on_connect(int rc) override {
        if (rc == 0) {
            std::cout << "[MQTT] 브로커 연결 성공" << std::endl;
            subscribe(nullptr, MQTT_COMMAND_TOPIC);
            std::cout << "[MQTT] 구독 완료: " << MQTT_COMMAND_TOPIC << std::endl;
        } else {
            std::cerr << "[MQTT] 연결 실패(rc=" << rc << ")" << std::endl;
        }
    }

    void on_message(const struct mosquitto_message* message) override {
        std::vector<uint8_t> req(
            (uint8_t*)message->payload,
            (uint8_t*)message->payload + message->payloadlen
        );

        const size_t UDS_OFFSET = 12;

        if (req.size() < UDS_OFFSET + 2) {
            std::cerr << "[MQTT] 잘못된 DoIP 프레임 (UDS 데이터 부족)" << std::endl;

            std::cout << "[Route] DoIP로 전송" << std::endl;
            auto doip_resp = sendDoipRequest(req);
            if (doip_resp)
                publish(nullptr, MQTT_RESPONSE_TOPIC, doip_resp->size(), doip_resp->data());
            return;
        }

        uint8_t sid = req[UDS_OFFSET];
        uint16_t did = 0;
        uint8_t subFunc = 0;

        if (sid == 0x19) {
            subFunc = req[UDS_OFFSET + 1];
            std::cout << "[Parser] SID=0x19 (ReadDTCInformation), SubFunction=0x"
                    << std::hex << (int)subFunc << std::dec << std::endl;
            std::cout << "[Route] DTC 요청 → DoIP로 전송" << std::endl;
            auto doip_resp = sendDoipRequest(req);
            if (doip_resp)
                publish(nullptr, MQTT_RESPONSE_TOPIC, doip_resp->size(), doip_resp->data());
            return;
        } else {
            did = (req[UDS_OFFSET + 1] << 8) | req[UDS_OFFSET + 2];
            std::cout << "[Parser] SID=0x" << std::hex << (int)sid
                    << " DID=0x" << std::setw(4) << std::setfill('0') << did << std::dec << std::endl;
        }

        std::cout << "[Parser] SID=0x" << std::hex << (int)sid
                << " DID=0x" << std::setw(4) << std::setfill('0') << did << std::dec << std::endl;

        std::optional<std::vector<uint8_t>> resp;

        // 특정 SID/DID 조합은 CAN-TP로 라우팅
        if ((sid == 0x22 && did == 0x1002) ||   // 조도센서
            (sid == 0x2F && did == 0x3000) ||
            (sid == 0x22 && did == 0x2002) ||
            (sid == 0x2E && did == 0x2002)) {   // 전조등 제어
            std::cout << "[Route] CAN-TP로 전송" << std::endl;

            // CAN-TP로는 UDS 부분만 전송 (헤더 제거)
            std::vector<uint8_t> uds_only(req.begin() + UDS_OFFSET, req.end());
            auto can_resp = sendCanTpRequest(uds_only);
            std::cout << "[Route] 전송 완료" << std::endl;

            if (can_resp) {
                // === 🟢 CAN 응답을 DoIP 헤더 포함 포맷으로 MQTT로 보냄 ===
                std::vector<uint8_t> doip_resp;
                std::cout << "[Route] 헤더 생성" << std::endl;
                // [DoIP 헤더 8바이트 구성]
                doip_resp.insert(doip_resp.end(), {0x03, 0xFC});  // VER, INV
                doip_resp.insert(doip_resp.end(), {0x80, 0x01});  // Payload Type (Diag Msg)
                uint32_t payload_len = 4 + can_resp->size();       // SA/TA(4) + UDS
                doip_resp.push_back((payload_len >> 24) & 0xFF);
                doip_resp.push_back((payload_len >> 16) & 0xFF);
                doip_resp.push_back((payload_len >> 8) & 0xFF);
                doip_resp.push_back(payload_len & 0xFF);

                // SA/TA (0E80/0201 고정)
                doip_resp.insert(doip_resp.end(), {0x0E, 0x80, 0x02, 0x01});

                // UDS 응답 붙이기
                doip_resp.insert(doip_resp.end(), can_resp->begin(), can_resp->end());

                print_hex("[Bridge] CAN→DoIP 변환 후 MQTT 발행: ", doip_resp);
                publish(nullptr, MQTT_RESPONSE_TOPIC, doip_resp.size(), doip_resp.data());
            }
            return;
        }

        // === 나머지는 그대로 DoIP ===
        std::cout << "[Route] DoIP로 전송" << std::endl;
        auto doip_resp = sendDoipRequest(req);
        if (doip_resp)
            publish(nullptr, MQTT_RESPONSE_TOPIC, doip_resp->size(), doip_resp->data());
    }
};

// ============================================================
// 🧩 main()
// ============================================================
int main() {
    mosqpp::lib_init();
    MqttBridge bridge("uds_gateway_rpi");

    std::cout << "[Main] MQTT 브로커 연결 중..." << std::endl;
    int rc = bridge.connect(MQTT_BROKER_IP, MQTT_PORT, 60);
    if (rc != MOSQ_ERR_SUCCESS) {
        std::cerr << "[Main] MQTT 연결 실패: " << mosqpp::strerror(rc) << std::endl;
        mosqpp::lib_cleanup();
        return 1;
    }

    bridge.loop_forever();
    mosqpp::lib_cleanup();
    return 0;
}

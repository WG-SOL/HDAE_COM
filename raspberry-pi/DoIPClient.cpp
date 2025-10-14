#include <iostream>
#include <vector>
#include <cstdint>
#include <string>
#include <iomanip>
#include <stdexcept>

// Platform-specific headers
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <cstring>
    #define SOCKET int
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket(s) close(s)
#endif

// --- DID 정의 ---
#define DID_TOF 0x0100

void print_hex(const std::string& prefix, const std::vector<uint8_t>& data) {
    std::cout << prefix;
    for (const auto& byte : data) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    }
    std::cout << std::dec << std::endl; // Switch back to decimal for normal output
}

int main() {
    const char* TC375_IP = "192.168.10.20";
    const int DOIP_PORT = 13400;

#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        return 1;
    }
#endif

    SOCKET client_socket = INVALID_SOCKET;
    try {
        client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (client_socket == INVALID_SOCKET) throw std::runtime_error("소켓 생성 실패.");
        std::cout << "1. 소켓 생성 완료." << std::endl;

        sockaddr_in server_addr{};
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(DOIP_PORT);
        inet_pton(AF_INET, TC375_IP, &server_addr.sin_addr);

        std::cout << "2. TC375 (" << TC375_IP << ":" << DOIP_PORT << ")에 연결을 시도합니다..." << std::endl;
        if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
            throw std::runtime_error("TCP 연결 실패.");
        }
        std::cout << "✅ TCP 연결 성공!" << std::endl;

        std::vector<uint8_t> route_request_msg = {
            0x03, 0xFC, 0x00, 0x05, 0x00, 0x00, 0x00, 0x07, // DoIP Header (Protocol Ver 0x03)
            0x0E, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00        // Payload
        };
        print_hex("3. 라우팅 활성화 요청 전송: ", route_request_msg);
        send(client_socket, (const char*)route_request_msg.data(), route_request_msg.size(), 0);

        std::vector<uint8_t> response_buf(1024);
        int bytes_received = recv(client_socket, (char*)response_buf.data(), response_buf.size(), 0);
        if (bytes_received <= 0) throw std::runtime_error("라우팅 활성화 응답 수신 실패.");
        
        response_buf.resize(bytes_received);
        print_hex("4. 라우팅 활성화 응답 수신: ", response_buf);

        if (response_buf.size() >= 13 && response_buf[2] == 0x00 && response_buf[3] == 0x06 && response_buf[12] == 0x10) {
            std::cout << "✅ 라우팅 활성화 성공!" << std::endl;

            // --- ToF 센서 값 요청 ---
            std::cout << "\n--- [ToF 센서] 값 요청 시작 ---" << std::endl;
            
            std::vector<uint8_t> diag_request_msg = {
                0x03, 0xFC, 0x80, 0x01, 0x00, 0x00, 0x00, 0x07, // DoIP Header (Payload Length 7)
                0x0E, 0x80,       // Source Address
                0x02, 0x01,       // Target Address
                0x22,             // UDS SID: ReadDataByIdentifier
                (uint8_t)(DID_TOF >> 8),
                (uint8_t)(DID_TOF & 0xFF)
            };
            print_hex("5. ToF 센서 값 요청 전송: ", diag_request_msg);
            send(client_socket, (const char*)diag_request_msg.data(), diag_request_msg.size(), 0);

            bytes_received = recv(client_socket, (char*)response_buf.data(), response_buf.size(), 0);
            if (bytes_received <= 0) throw std::runtime_error("진단 응답 수신 실패.");
            
            response_buf.resize(bytes_received);
            print_hex("6. ToF 센서 값 응답 수신: ", response_buf);

            if (response_buf.size() >= 13 && response_buf[8] == 0x62) {
                uint16_t resp_did = (static_cast<uint16_t>(response_buf[9]) << 8) | response_buf[10];
                if (resp_did == DID_TOF) {
                    uint16_t value = (static_cast<uint16_t>(response_buf[11]) << 8) | response_buf[12];
                    std::cout << "✅ 수신된 거리 값: " << value << " mm" << std::endl;
                }
            }
        } else {
            std::cout << "❌ 라우팅 활성화 실패." << std::endl;
        }

    } catch (const std::runtime_error& e) {
        std::cerr << "❌ 오류 발생: " << e.what() << std::endl;
    }

    if (client_socket != INVALID_SOCKET) {
        closesocket(client_socket);
        std::cout << "\n소켓을 닫았습니다." << std::endl;
    }

#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}
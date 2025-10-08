#ifndef CAN_TP_SENDER_H
#define CAN_TP_SENDER_H

#include <vector>
#include <string>

class CanTpSender {
public:
    /**
     * @brief 생성자. CAN 인터페이스를 초기화합니다.
     * @param interfaceName 사용할 CAN 인터페이스 이름 (예: "can0")
     */
    CanTpSender(const std::string& interfaceName);

    /**
     * @brief 소멸자. CAN 소켓을 닫습니다.
     */
    ~CanTpSender();

    /**
     * @brief CAN TP 메시지를 전송합니다.
     * @param data 전송할 데이터 바이트 벡터
     * @return 성공 시 true, 실패 시 false
     */
    bool sendMessage(const std::vector<unsigned char>& data);

private:
    int socket_fd; // 소켓 파일 디스크립터
    bool is_initialized;

    /**
     * @brief CAN 소켓을 초기화하는 내부 함수
     * @param interfaceName CAN 인터페이스 이름
     * @return 성공 시 true, 실패 시 false
     */
    bool init(const std::string& interfaceName);
};

#endif // CAN_TP_SENDER_H

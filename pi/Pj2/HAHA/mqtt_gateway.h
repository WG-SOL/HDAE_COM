#ifndef MQTT_GATEWAY_H
#define MQTT_GATEWAY_H

#include <mosquittopp.h>
#include "can_tp_sender.h" // CanTpSender 클래스를 사용하기 위해 포함

class MqttCanGateway : public mosqpp::mosquittopp {
public:
    /**
     * @brief 생성자
     * @param id MQTT 클라이언트 ID
     * @param sender CAN 메시지를 보낼 CanTpSender 객체의 참조
     */
    MqttCanGateway(const char* id, CanTpSender& sender);

    // 연결 성공 시 호출되는 콜백
    void on_connect(int rc) override;

    // 메시지 수신 시 호출되는 콜백
    void on_message(const struct mosquitto_message* message) override;

private:
    CanTpSender& can_sender; // CAN 메시지를 보내기 위한 CanTpSender 객체 참조

    /**
     * @brief 16진수 문자열을 바이트 벡터로 변환하는 헬퍼 함수
     * @param hex_str 변환할 16진수 문자열s
     * @param out_vec 변환된 바이트가 저장될 벡터
     * @return 성공 시 true, 실패 시 false
     */
    bool hex_string_to_bytes(const std::string& hex_str, std::vector<unsigned char>& out_vec);
};

#endif // MQTT_GATEWAY_H

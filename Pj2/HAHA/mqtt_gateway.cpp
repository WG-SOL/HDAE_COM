#include "mqtt_gateway.h"
#include <iostream>
#include <vector>

MqttCanGateway::MqttCanGateway(const char* id, CanTpSender& sender)
    : mosqpp::mosquittopp(id), can_sender(sender) {}

void MqttCanGateway::on_connect(int rc) {
    if (rc == 0) {
        std::cout << "[MQTT] 브로커에 연결되었습니다. 토픽 구독을 시작합니다." << std::endl;
        // 연결 성공 시 'vehicle/command' 토픽을 구독
        subscribe(NULL, "vehicle/command");
    } else {
        std::cerr << "[MQTT] 브로커 연결에 실패했습니다." << std::endl;
    }
}

void MqttCanGateway::on_message(const struct mosquitto_message* message) {
    // payloadlen으로 길이를 확인하여 문자열로 변환
    std::string payload(static_cast<char*>(message->payload), message->payloadlen);
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "[MQTT] 토픽 '" << message->topic << "'에서 메시지 수신: " << payload << std::endl;

    // 수신된 16진수 문자열을 바이트 벡터로 변환
    std::vector<unsigned char> can_data;
    if (hex_string_to_bytes(payload, can_data)) {
        std::cout << "[CAN] " << can_data.size() << " 바이트를 CAN-TP를 통해 전송합니다..." << std::endl;
        // 변환된 데이터를 CAN으로 전송
        if (can_sender.sendMessage(can_data)) {
            std::cout << "[CAN] 전송 성공." << std::endl;
        } else {
            std::cerr << "[CAN] 전송 실패." << std::endl;
        }
    } else {
        std::cerr << "[MQTT] 페이로드 형식이 잘못되었습니다. (16진수 문자열 필요)" << std::endl;
    }
}

bool MqttCanGateway::hex_string_to_bytes(const std::string& hex_str, std::vector<unsigned char>& out_vec) {
    if (hex_str.length() % 2 != 0) {
        // 16진수 문자열은 항상 짝수 길이를 가져야 함
        return false;
    }

    out_vec.clear();
    for (size_t i = 0; i < hex_str.length(); i += 2) {
        try {
            std::string byte_str = hex_str.substr(i, 2);
            unsigned char byte = static_cast<unsigned char>(std::stoul(byte_str, nullptr, 16));
            out_vec.push_back(byte);
        } catch (const std::exception& e) {
            // 변환 실패 (숫자가 아닌 문자가 포함된 경우 등)
            return false;
        }
    }
    return true;
}

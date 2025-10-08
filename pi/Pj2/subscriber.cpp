#define _CRT_SECURE_NO_WARNINGS // Windows에서 일부 C 함수 경고를 무시하기 위해 필요
#include <iostream>
#include <mosquittopp.h>

// mosquittopp 클래스를 상속받아 자신만의 Subscriber 클래스를 정의
class MqttSubscriber : public mosqpp::mosquittopp {
public:
    MqttSubscriber(const char* id) : mosqpp::mosquittopp(id) {}

    // 연결 성공 시 호출되는 콜백 함수
    void on_connect(int rc) override {
        if (rc == 0) {
            std::cout << "Subscriber: Connected to broker." << std::endl;
            // 연결 성공 시 토픽 구독
            subscribe(NULL, "hello/topic");
        } else {
            std::cout << "Subscriber: Connection failed." << std::endl;
        }
    }

    // 메시지 수신 시 호출되는 콜백 함수
    void on_message(const struct mosquitto_message* message) override {
        // payloadlen으로 길이를 확인하여 문자열로 변환
        std::string payload(static_cast<char*>(message->payload), message->payloadlen);
        std::cout << "Received message on topic '" << message->topic << "': " << payload << std::endl;
    }
};

int main() {
    mosqpp::lib_init(); // 라이브러리 초기화

    MqttSubscriber subscriber("my_subscriber");
    
    // 로컬호스트 브로커에 연결 (IP 주소로 변경 가능)
    int rc = subscriber.connect("10.174.91.222", 1883, 60);
    if (rc != MOSQ_ERR_SUCCESS) {
        std::cerr << "Error: " << mosqpp::strerror(rc) << std::endl;
        return 1;
    }

    // 메시지 수신을 위해 네트워크 루프 실행 (종료되지 않고 계속 대기)
    subscriber.loop_forever();

    mosqpp::lib_cleanup(); // 라이브러리 정리
    return 0;
}

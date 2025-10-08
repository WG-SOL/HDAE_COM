#include <iostream>
#include "mqtt_gateway.h"
#include "can_tp_sender.h"
#include "config.h"
int main() {
    // 1. Mosquitto 라이브러리 초기화
    mosqpp::lib_init();

    // 2. CAN 송신 객체 생성
    CanTpSender can_sender(CAN_INTERFACE);

    // 3. MQTT 게이트웨이 객체 생성 (CAN 송신 객체를 전달)
    MqttCanGateway gateway(MQTT_CLIENT_ID, can_sender);
    
    // 4. MQTT 브로커에 연결
    int rc = gateway.connect(MQTT_BROKER_IP, MQTT_BROKER_PORT, 60);
    if (rc != MOSQ_ERR_SUCCESS) {
        std::cerr << "MQTT 브로커 연결 실패: " << mosqpp::strerror(rc) << std::endl;
        mosqpp::lib_cleanup();
        return 1;
    }

    // 5. MQTT 네트워크 루프 실행 (메시지 수신을 위해 계속 대기)
    // on_message 콜백 함수에서 모든 작업이 처리됩니다.
    std::cout << "MQTT-CAN 게이트웨이가 시작되었습니다. 메시지를 기다립니다..." << std::endl;
    gateway.loop_forever();

    // 6. 프로그램 종료 시 라이브러리 정리
    mosqpp::lib_cleanup();
    return 0;
}

#ifndef CONFIG_H
#define CONFIG_H

#include <string>

// MQTT 브로커 정보
const std::string MQTT_BROKER_IP = "10.174.91.222";
constexpr int     MQTT_BROKER_PORT = 1883;
const std::string MQTT_TOPIC = "vehicle/command";

// MQTT 클라이언트 정보
const std::string MQTT_CLIENT_ID = "mqtt_to_can_gateway";

// CAN 인터페이스 정보
const std::string CAN_INTERFACE = "can0";

#endif // CONFIG_H

#ifndef CONFIG_H
#define CONFIG_H

#include <string>

// 'extern' 키워드를 사용해 변수가 다른 곳에 정의되어 있음을 알림
extern const std::string MQTT_BROKER_IP;
extern const int     MQTT_BROKER_PORT;

const std::string MQTT_TOPIC;

// MQTT 클라이언트 정보
const std::string MQTT_CLIENT_ID;

// CAN 인터페이스 정보
const std::string CAN_INTERFACE;
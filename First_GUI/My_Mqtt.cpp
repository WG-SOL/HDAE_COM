#define _CRT_SECURE_NO_WARNINGS

#include "My_Mqtt.h"
#include <filesystem>
namespace fs = std::filesystem;

MqttHandler::MqttHandler(const char* id, QObject *parent)
    : QObject(parent), mosquittopp(id)
{
    mosqpp::lib_init();
    loop_start();
}

MqttHandler::~MqttHandler()
{
    loop_stop(true);
    mosqpp::lib_cleanup();
}

void MqttHandler::connectToBroker(const char* host, int port)
{
    connect_async(host, port, 60);
}

void MqttHandler::publishMessage(const std::string& topic, const std::string& message)
{
    publish(NULL, topic.c_str(), message.length(), message.c_str());
}

void MqttHandler::sendFile(const std::string& topic, const std::string& filePath)
{
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "[MQTT] 파일을 열 수 없습니다: " << filePath << std::endl;
        return;
    }

    std::vector<uint8_t> buffer((std::istreambuf_iterator<char>(file)), {});
    const size_t CHUNK_SIZE = 4096;

    std::cout << "[MQTT] OTA 전송 시작: " << buffer.size() << " bytes" << std::endl;

    size_t offset = 0;
    while (offset < buffer.size()) {
        size_t len = std::min(CHUNK_SIZE, buffer.size() - offset);
        int rc = publish(nullptr, topic.c_str(), len, buffer.data() + offset, 1, false);
        if (rc != MOSQ_ERR_SUCCESS) {
            std::cerr << "[MQTT] publish 실패 (offset=" << offset
                      << "): " << mosqpp::strerror(rc) << std::endl;
            break;
        }
        offset += len;
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    const char* doneMsg = "EOF";
    publish(nullptr, "ota/firmware/done", strlen(doneMsg), doneMsg, 1, false);
    std::cout << "[MQTT] OTA 전송 완료" << std::endl;
}

void MqttHandler::on_connect(int rc)
{
    if (rc == 0) {
        std::cout << "MQTT Handler: Connected to broker." << std::endl;
        m_isConnected = true;

        // ★★★ 필요한 모든 응답 토픽을 여기서 구독 ★★★
        subscribe(NULL, "dtc/response");
        subscribe(NULL, "response/topic"); // 센서 데이터 응답 토픽

        emit connected();
    } else {
        std::cout << "MQTT Handler: Connection failed." << std::endl;
        m_isConnected = false;
    }
}

void MqttHandler::on_message(const struct mosquitto_message* message)
{
    if (message) {
        // [변경] 수신된 데이터를 std::string 대신 QString과 QByteArray로 변환
        QString topic_qstr = QString::fromStdString(message->topic);
        QByteArray payload_qba(static_cast<char*>(message->payload), message->payloadlen);

        // [변경] 수정된 시그널을 Qt 타입으로 emit
        emit messageReceived(topic_qstr, payload_qba);
    }
}

void MqttHandler::on_disconnect(int rc)
{
    std::cout << "MQTT Handler: Disconnected." << std::endl;
    m_isConnected = false;
    emit disconnected();
}

bool MqttHandler::isConnected() const
{
    return m_isConnected;
}

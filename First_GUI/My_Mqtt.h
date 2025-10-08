#ifndef MY_MQTT_H
#define MY_MQTT_H
#include <iostream>
#include <mosquittopp.h>
#include <thread> // std::this_thread
#include <chrono> // std::chrono
#include <QObject>
#include <string>
#include <fstream>
#include <vector>
class MqttHandler : public QObject, public mosqpp::mosquittopp {
    Q_OBJECT // Qt의 시그널/슬롯을 사용하기 위해 필수

    public:
        explicit MqttHandler(const char* id, QObject *parent = nullptr);
        ~MqttHandler();

        void connectToBroker(const char* host, int port = 1883);
        void publishMessage(const std::string& topic, const std::string& message);
        void sendFile(const std::string& topic, const std::string& file_path);
        bool isConnected() const;

    signals:
        void connected();
        void disconnected();

    private:
        // mosquittopp의 가상 함수들을 오버라이드
        void on_connect(int rc) override;
        void on_disconnect(int rc) override;
        bool m_isConnected = false;
};

#endif // MY_MQTT_H

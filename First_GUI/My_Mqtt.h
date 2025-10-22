#ifndef MY_MQTT_H
#define MY_MQTT_H

#include <iostream>
#include <mosquittopp.h>
#include <string>
#include <fstream>
#include <vector>
#include <thread>
#include <chrono>
#include <QObject>
#include <QString>
#include <QByteArray>

class MqttHandler : public QObject, public mosqpp::mosquittopp {
    Q_OBJECT

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
    // [변경] std::string 대신 QString과 QByteArray를 사용하도록 시그널 수정
    void messageReceived(const QString& topic, const QByteArray& payload);

private:
    void on_connect(int rc) override;
    void on_disconnect(int rc) override;
    void on_message(const struct mosquitto_message* message) override;

    bool m_isConnected = false;
};

#endif // MY_MQTT_H

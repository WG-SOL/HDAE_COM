#ifndef MODIFYWINDOW_H
#define MODIFYWINDOW_H

#include <QWidget>
#include "My_Mqtt.h" // MqttHandler

// Qt 클래스 전방 선언
class QLineEdit;
class QPushButton;

/**
 * @class ModifyWindow
 * @brief DID 값을 ECU에 쓰기 위한 UI 창 클래스입니다.
 */
class ModifyWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ModifyWindow(MqttHandler* handler, QWidget* parent = nullptr);

private slots:
    void onSendClicked();

private:
    /**
     * @brief UDS WriteDataByIdentifier (0x2E) 프레임을 생성합니다.
     * @param did 쓸 데이터의 DID
     * @param value 쓸 2바이트 값
     * @return 생성된 UDS 프레임 (5 바이트)
     */
    QByteArray buildUdsWriteFrame(quint16 did, quint16 value);

    /**
     * @brief DoIP 헤더와 UDS 프레임을 결합하여 완전한 MQTT 메시지를 전송합니다.
     * @param did 쓸 데이터의 DID
     * @param value 쓸 2바이트 값
     */
    void sendWriteRequest(quint16 did, quint16 value);

    // --- 멤버 변수 ---
    MqttHandler* m_mqttHandler;
    QLineEdit* abeEdit;
    QLineEdit* headlightEdit;
    QPushButton* sendButton;
};

#endif // MODIFYWINDOW_H

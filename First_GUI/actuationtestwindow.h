#ifndef ACTUATIONTESTWINDOW_H
#define ACTUATIONTESTWINDOW_H

#include <QWidget>
#include "My_Mqtt.h" // MqttHandler 클래스 헤더

// Qt 클래스 전방 선언
class QGroupBox;
class QComboBox;
class QSpinBox;
class QPushButton;

/**
 * @class ActuationTestWindow
 * @brief 모터, LED 등의 장치를 강제로 구동하기 위한 테스트 창 클래스입니다.
 *
 * 사용자가 입력한 값을 기반으로 UDS 형식의 제어 메시지를 생성하여 MQTT로 전송합니다.
 */
class ActuationTestWindow : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief 생성자
     * @param mqttHandler MQTT 통신을 담당하는 핸들러 객체에 대한 포인터
     * @param parent 부모 위젯
     */
    explicit ActuationTestWindow(MqttHandler *mqttHandler, QWidget *parent = nullptr);
    ~ActuationTestWindow();

private slots:
    // --- UI 컨트롤을 위한 슬롯 ---
    void onRunMotorClicked();
    void onLedOnClicked();
    void onLedOffClicked();

private:
    /**
     * @brief 사용자 인터페이스를 생성하고 초기화합니다.
     */
    void setupUi();

    // --- 멤버 변수 ---
    MqttHandler *m_mqttHandler;

    // 모터 제어 UI
    QComboBox *motorDirectionCombo;
    QSpinBox *motorPwmSpinBox;
    QPushButton *runMotorButton;

    // LED 제어 UI
    QComboBox *ledNumberCombo;
    QPushButton *ledOnButton;
    QPushButton *ledOffButton;
};

#endif // ACTUATIONTESTWINDOW_H

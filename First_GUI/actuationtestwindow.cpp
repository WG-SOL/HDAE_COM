#include "actuationtestwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QMessageBox>
#include <QDebug>

ActuationTestWindow::ActuationTestWindow(MqttHandler *mqttHandler, QWidget *parent)
    : QWidget(parent)
    , m_mqttHandler(mqttHandler)
{
    setupUi();
}

ActuationTestWindow::~ActuationTestWindow()
{
}

void ActuationTestWindow::setupUi()
{
    setWindowTitle("Forced Actuation Test");
    this->resize(500, 350);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // --- 모터 제어 그룹 ---
    QGroupBox *motorGroup = new QGroupBox("Motor Control");
    mainLayout->addWidget(motorGroup);

    QGridLayout *motorLayout = new QGridLayout(motorGroup);
    motorLayout->setSpacing(10);

    motorLayout->addWidget(new QLabel("Direction:"), 0, 0);
    motorDirectionCombo = new QComboBox();
    motorDirectionCombo->addItems({"0 (Forward)", "1 (Reverse)"});
    motorLayout->addWidget(motorDirectionCombo, 0, 1);

    motorLayout->addWidget(new QLabel("PWM (0-250):"), 1, 0);
    motorPwmSpinBox = new QSpinBox();
    motorPwmSpinBox->setRange(0, 250);
    motorLayout->addWidget(motorPwmSpinBox, 1, 1);

    runMotorButton = new QPushButton("Run Motor");
    motorLayout->addWidget(runMotorButton, 2, 0, 1, 2); // 버튼이 두 컬럼을 차지하도록 설정

    // --- LED 제어 그룹 ---
    QGroupBox *ledGroup = new QGroupBox("LED Control");
    mainLayout->addWidget(ledGroup);

    QGridLayout *ledLayout = new QGridLayout(ledGroup);
    ledLayout->setSpacing(10);

    ledLayout->addWidget(new QLabel("LED Number:"), 0, 0);
    ledNumberCombo = new QComboBox();
    ledNumberCombo->addItems({"1", "2", "3"});
    ledLayout->addWidget(ledNumberCombo, 0, 1, 1, 2);

    ledOnButton = new QPushButton("ON");
    ledLayout->addWidget(ledOnButton, 1, 1);

    ledOffButton = new QPushButton("OFF");
    ledLayout->addWidget(ledOffButton, 1, 2);

    mainLayout->addStretch(); // 위젯들을 위로 정렬

    // 시그널-슬롯 연결
    connect(runMotorButton, &QPushButton::clicked, this, &ActuationTestWindow::onRunMotorClicked);
    connect(ledOnButton, &QPushButton::clicked, this, &ActuationTestWindow::onLedOnClicked);
    connect(ledOffButton, &QPushButton::clicked, this, &ActuationTestWindow::onLedOffClicked);
}

void ActuationTestWindow::onRunMotorClicked()
{
    if (!m_mqttHandler || !m_mqttHandler->isConnected()) {
        QMessageBox::warning(this, "Connection Error", "MQTT client is not connected.");
        return;
    }

    // UDS InputOutputControlByIdentifier (0x2F) 요청 헤더
    const QString udsHeader = "03FC80010000000A0E8002012F";
    // 참고: 모터 제어를 위한 DID는 0x3001로 가정합니다. (실제 값으로 변경 필요)
    const QString did = "4000";
    // 참고: 제어 옵션은 0x03 (returnControlToECU)로 가정합니다.
    const QString controlOption = "03";

    // 사용자 입력 값 가져오기
    quint8 direction = motorDirectionCombo->currentIndex(); // 0 또는 1
    quint8 pwm = motorPwmSpinBox->value();

    // DID, 제어 옵션, 방향, PWM 값을 16진수 문자열로 변환
    QString directionStr = QString("%1").arg(direction, 2, 16, QChar('0'));
    QString pwmStr = QString("%1").arg(pwm, 2, 16, QChar('0'));

    // 전체 메시지 조립
    QString hexMessage = udsHeader + did + controlOption + directionStr + pwmStr;

    // 16진수 문자열을 바이트로 변환하여 전송
    QByteArray payloadBytes = QByteArray::fromHex(hexMessage.toLatin1());
    m_mqttHandler->publishMessage("request/topic", payloadBytes.toStdString());

    qDebug() << "Sent Motor Control Command:" << hexMessage.toUpper();
    QMessageBox::information(this, "Command Sent", "Motor control command has been sent.");
}


void ActuationTestWindow::onLedOnClicked()
{
    if (!m_mqttHandler || !m_mqttHandler->isConnected()) {
        QMessageBox::warning(this, "Connection Error", "MQTT client is not connected.");
        return;
    }

    // UDS InputOutputControlByIdentifier (0x2F) 요청 헤더
    const QString udsHeader = "03FC80010000000A0E8002012F";
    // 참고: LED 제어를 위한 DID는 0x3002로 가정합니다.
    const QString did = "3000";
    const QString controlOption = "03";

    quint8 ledNum = ledNumberCombo->currentText().toUInt();
    quint8 ledState = 1; // ON

    QString ledStateStr = QString("%1").arg(ledState, 2, 16, QChar('0'));

    QString hexMessage = udsHeader + did + controlOption + ledStateStr;
    QByteArray payloadBytes = QByteArray::fromHex(hexMessage.toLatin1());
    m_mqttHandler->publishMessage("request/topic", payloadBytes.toStdString());

    qDebug() << "Sent LED ON Command:" << hexMessage.toUpper();
    QMessageBox::information(this, "Command Sent", "LED ON command has been sent.");
}

void ActuationTestWindow::onLedOffClicked()
{
    if (!m_mqttHandler || !m_mqttHandler->isConnected()) {
        QMessageBox::warning(this, "Connection Error", "MQTT client is not connected.");
        return;
    }

    const QString udsHeader = "03FC80010000000A0E8002012F";
    const QString did = "3000";
    const QString controlOption = "03";

    quint8 ledState = 0; // OFF

    QString ledStateStr = QString("%1").arg(ledState, 2, 16, QChar('0'));

    QString hexMessage = udsHeader + did + controlOption + ledStateStr;
    QByteArray payloadBytes = QByteArray::fromHex(hexMessage.toLatin1());
    m_mqttHandler->publishMessage("request/topic", payloadBytes.toStdString());

    qDebug() << "Sent LED OFF Command:" << hexMessage.toUpper();
    QMessageBox::information(this, "Command Sent", "LED OFF command has been sent.");
}

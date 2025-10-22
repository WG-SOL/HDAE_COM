#include "ScanSelectionWindow.h"
#include "dtcwindow.h"
#include "actuationtestwindow.h"
#include "sensordatawindow.h"
#include "modifywindow.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QFont>
#include <QDebug>
#include <QIcon>
#include <QLabel>
#include <QFile>
#include <QStyle>
#include <QMessageBox>
ScanSelectionWindow::ScanSelectionWindow(MqttHandler *mqttHandler, QWidget *parent)
    : QWidget(parent)
    , m_mqttHandler(mqttHandler), m_isExtendedSession(false)
{
    setupUi();
    // ★★★ 자식 윈도우를 생성할 때 MqttHandler를 전달합니다. ★★★
    m_dtcWindow = new DTCWindow(m_mqttHandler, nullptr);
    m_sensorDataWindow = new SensorDataWindow(m_mqttHandler, nullptr);
    m_actuationTestWindow = new ActuationTestWindow(m_mqttHandler, nullptr);
    const QString StartudsHeader = "03FC0005000000070E800000000000";
    QString hexMessage = StartudsHeader;

    // 3. 16진수 문자열을 실제 바이트 데이터(QByteArray)로 변환합니다.
    QByteArray payloadBytes = QByteArray::fromHex(hexMessage.toLatin1());

    // 4. 변환된 바이트 데이터를 MQTT로 전송합니다.
    // 참고: MqttHandler의 publishMessage가 std::string을 받도록 되어있으므로 변환합니다.
    m_mqttHandler->publishMessage("request/topic", payloadBytes.toStdString());
    m_modifyWindow = new ModifyWindow(m_mqttHandler, nullptr);
}

ScanSelectionWindow::~ScanSelectionWindow()
{
    delete m_dtcWindow;
    delete m_sensorDataWindow;
    delete m_actuationTestWindow;
    delete m_modifyWindow;
}

void ScanSelectionWindow::setupUi()
{
    this->resize(800, 480);
    this->setObjectName("ScanSelectionWindow");

    // 1. ScanSelectionWindow를 꽉 채울 최상위 레이아웃을 만듭니다.
    QVBoxLayout* topLevelLayout = new QVBoxLayout(this);
    topLevelLayout->setContentsMargins(0, 0, 0, 0);

    // 2. 배경 이미지를 담당할 컨테이너 위젯을 만듭니다.
    QWidget* backgroundContainer = new QWidget(this);
    backgroundContainer->setObjectName("backgroundContainer"); // QSS에서 이 이름으로 스타일을 적용합니다!

    // 3. 최상위 레이아웃에 배경 컨테이너를 추가합니다.
    topLevelLayout->addWidget(backgroundContainer);

    // 4. ★★★ 기존 mainLayout의 부모를 'this'가 아닌 'backgroundContainer'로 변경합니다. ★★★
    QVBoxLayout *mainLayout = new QVBoxLayout(backgroundContainer);

    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    QWidget* headerContainer = new QWidget(this);
    headerContainer->setObjectName("headerContainer"); // QSS에서 이 위젯을 #headerContainer로 지칭합니다.

    QHBoxLayout *headerLayout = new QHBoxLayout(headerContainer);
    headerLayout->setContentsMargins(20, 0, 20, 0); // 좌우 여백 20px

    // 3. 제목 텍스트 라벨을 생성합니다.
    QLabel *titleTextLabel = new QLabel("Scan", this);

    sessionButton = new QPushButton(this);
    sessionButton->setObjectName("sessionButton"); // QSS에서 특정하기 위한 이름


    headerLayout->addWidget(titleTextLabel); // 왼쪽: "Scan" 텍스트
    headerLayout->addStretch();              // 중앙: 신축성 있는 빈 공간
    headerLayout->addWidget(sessionButton);  // 오른쪽: 세션 버튼

    mainLayout->addWidget(headerContainer);


    dtcButton = new QPushButton("Error Code", this);
    sensorDataButton = new QPushButton("Sensor Data", this);
    actuationButton = new QPushButton("Forced Active", this);
    modifyButton = new QPushButton("Data Modify", this);


    // image
//    dtcButton->setIcon(QIcon(":/icons/dtc_icon.png"));
//    sensorDataButton->setIcon(QIcon(":/icons/sensordata_icon.png"));
//    actuationButton->setIcon(QIcon(":/icons/actuation_icon.png"));
//    ecuInfoButton->setIcon(QIcon(":/icons/ecuinfo_icon.png"));
//    specialFuncButton->setIcon(QIcon(":/icons/special_icon.png"));
//    // ★★★ 아이콘 크기 설정
//    QSize iconSize(32, 32);
//    dtcButton->setIconSize(iconSize);
//    sensorDataButton->setIconSize(iconSize);
//    actuationButton->setIconSize(iconSize);
//    ecuInfoButton->setIconSize(iconSize);
//    specialFuncButton->setIconSize(iconSize);


    dtcButton->setMinimumHeight(60);
    sensorDataButton->setMinimumHeight(60);
    actuationButton->setMinimumHeight(60);
    modifyButton->setMinimumHeight(60);


    // ★★★ 2. 버튼들만 담을 새로운 '중첩 레이아웃' 생성 ★★★
    QVBoxLayout *buttonLayout = new QVBoxLayout();
    buttonLayout->setSpacing(0);
    // ★★★ 이 레이아웃에만 좌우 여백을 20px로 설정 ★★★
    buttonLayout->setContentsMargins(80, 20, 80, 0);

    buttonLayout->addWidget(dtcButton);
    buttonLayout->addWidget(sensorDataButton);
    buttonLayout->addWidget(actuationButton);
    buttonLayout->addWidget(modifyButton);

    mainLayout->addLayout(buttonLayout);
    mainLayout->addStretch(); // 버튼들을 위로 정렬

    connect(dtcButton, &QPushButton::clicked, this, &ScanSelectionWindow::openDTCWindow);
    connect(sensorDataButton, &QPushButton::clicked, this, &ScanSelectionWindow::openSensorDataWindow);
    connect(actuationButton, &QPushButton::clicked, this, &ScanSelectionWindow::openActuationTestWindow);
    connect(sessionButton, &QPushButton::clicked, this, &ScanSelectionWindow::onSessionButtonClicked);
    connect(modifyButton, &QPushButton::clicked, this, &ScanSelectionWindow::openModifyWindow);

    updateButtonStates();
}

void ScanSelectionWindow::onSessionButtonClicked()
{
    if (!m_mqttHandler || !m_mqttHandler->isConnected()) {
        QMessageBox::warning(this, "Connection Error", "MQTT client is not connected.");
        return;
    }

    // 1. 세션 상태를 먼저 토글합니다.
    m_isExtendedSession = !m_isExtendedSession;

    // 2. UDS DiagnosticSessionControl(0x10) 요청 메시지를 정의합니다.
    // 헤더: 03FC8001 00000006(길이) 0E800201(주소)
    // 데이터: 10(SID) + 01(Default) 또는 03(Extended)
    const QString udsHeader = "03FC8001000000060E80020110";
    QString sessionType;

    if (m_isExtendedSession) {
        sessionType = "03"; // Extended Diagnostic Session
    } else {
        sessionType = "01"; // Default Session
    }

    // 3. 완전한 16진수 메시지를 생성하고 바이트로 변환합니다.
    QString hexMessage = udsHeader + sessionType;
    QByteArray payloadBytes = QByteArray::fromHex(hexMessage.toLatin1());

    // 4. MQTT로 UDS 메시지를 전송합니다.
    m_mqttHandler->publishMessage("request/topic", payloadBytes.toStdString());
    qDebug() << "Sent Session Change Command:" << hexMessage.toUpper();

    // 5. 변경된 상태를 UI(버튼 활성화/비활성화, 색상)에 반영합니다.
    updateButtonStates();
}

void ScanSelectionWindow::updateButtonStates()
{
    if (m_isExtendedSession) {
        // --- Extended 세션 상태 ---
        sessionButton->setText("Session: Extended");
        sessionButton->setProperty("sessionState", "extended"); // QSS를 위한 속성 설정

        dtcButton->setEnabled(true);
        sensorDataButton->setEnabled(true);
        actuationButton->setEnabled(true);
        modifyButton->setEnabled(true);
    } else {
        // --- Normal 세션 상태 ---
        sessionButton->setText("Session: Normal");
        sessionButton->setProperty("sessionState", "normal"); // QSS를 위한 속성 설정

        dtcButton->setEnabled(true);
        sensorDataButton->setEnabled(true);
        actuationButton->setEnabled(false);
        modifyButton->setEnabled(false);
    }
    style()->unpolish(sessionButton);
    style()->polish(sessionButton);
}

void ScanSelectionWindow::openDTCWindow()
{
    m_dtcWindow->show();
}

void ScanSelectionWindow::openSensorDataWindow()
{
    m_sensorDataWindow->show();
}

void ScanSelectionWindow::openActuationTestWindow()
{
    m_actuationTestWindow->show();
}


void ScanSelectionWindow::openModifyWindow()
{
    m_modifyWindow->show();
}

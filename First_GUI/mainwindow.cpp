#include "mainwindow.h"
#include "dtcwindow.h" // DTC 창 헤더 포함
#include <QGridLayout>
#include <QWidget>
#include <QIcon>
#include <QResizeEvent>
#include "Ipconfig.h"
#include <QMessageBox>
#include <QFileDialog> // 파일 대화상자를 위해 필요

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), dtcWindow(nullptr), connectionTimer(nullptr),progressDialog(nullptr), isShowingDialog(false) // dtcWindow 포인터를 nullptr로 초기화
{

    // this : Parent
    // Centralwidget : Main Widget
    // GirdLayout : Arrangement
    QWidget *centralWidget = new QWidget(this);
    QGridLayout *mainLayout = new QGridLayout(centralWidget);

    setCentralWidget(centralWidget);
    setWindowTitle("RC Car PC JINDAN");
    this->resize(800, 480);

    // Create Button
    QToolButton *savedDataButton = new QToolButton();
    scanTechButton = new QToolButton();
    OTAButton = new QToolButton();

    // For QSS
    savedDataButton->setObjectName("savedDataButton");
    OTAButton->setObjectName("OTAButton");
    scanTechButton->setObjectName("scanTechButton");
    this->setObjectName("MainWindow");

    savedDataButton->setText("SavedData");
    scanTechButton->setText("Scan");
    OTAButton->setText("OTA UPDATE");


    QSize buttonSize(120, 120);
    savedDataButton->setFixedSize(buttonSize);
    scanTechButton->setFixedSize(buttonSize);
    OTAButton->setFixedSize(buttonSize);


    // Icon Image Set
    savedDataButton->setIcon(QIcon(":/images/data_saved.png"));
    scanTechButton->setIcon(QIcon(":/images/scan_tech.png"));
    savedDataButton->setIconSize(QSize(64, 64));
    scanTechButton->setIconSize(QSize(64, 64));
    OTAButton->setIconSize(QSize(64, 64));


    // Text Under Icon Set
    savedDataButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    scanTechButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    OTAButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);


    // Button Style
    QSizePolicy policy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    savedDataButton->setSizePolicy(policy);
    scanTechButton->setSizePolicy(policy);
    OTAButton->setSizePolicy(policy);

    // Attach Button
    mainLayout->addWidget(savedDataButton, 0, 0);
    mainLayout->addWidget(OTAButton, 0, 1);
    mainLayout->addWidget(scanTechButton, 0, 2);

    // brightness control
    overlay = new QWidget(centralWidget); // centralWidget 위에 생성
    overlay->setStyleSheet("background-color: rgba(0, 0, 0, 50);"); // Red, Green, Blue, Brightness
    overlay->setAttribute(Qt::WA_TransparentForMouseEvents); // Mouse Control
    overlay->setGeometry(centralWidget->geometry());
    overlay->show();

    // --- MQTT ---
    mqttHandler = new MqttHandler("MyQtClient", this);

    // MqttHandler signal action
    // send object, signal, action object, Function
    connect(mqttHandler, &MqttHandler::connected, this, &MainWindow::onMqttConnected);
    connect(mqttHandler, &MqttHandler::disconnected, this, &MainWindow::onMqttDisconnected);

    QTimer::singleShot(0, this, &MainWindow::startMqttConnection);

    // "스캔테크" 버튼 클릭 시 DTC 창을 열도록 시그널-슬롯 연결
    connect(scanTechButton, &QToolButton::clicked, this, &MainWindow::openDTCWindow);
    connect(OTAButton, &QToolButton::clicked, this, &MainWindow::onOtaButtonClicked);
}

MainWindow::~MainWindow() {

}

// DTC 창을 여는 슬롯 함수 구현
void MainWindow::openDTCWindow() {
    //Mqtt Command
    std::cout << "Scan button clicked. Publishing MQTT message..." << std::endl;
    mqttHandler->publishMessage("hello/topic", "READ_DTC");

    if (!dtcWindow) {
        dtcWindow = new DTCWindow();
    }
    dtcWindow->show(); // 창 보여주기
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);
    if(overlay) {
        // 오버레이 위젯의 크기를 centralWidget 크기와 동일하게 맞춰줍니다.
        overlay->setGeometry(centralWidget()->geometry());
    }
}

void MainWindow::onMqttConnected() {
    std::cout << "MainWindow: Received connected signal from MqttHandler." << std::endl;
    if (connectionTimer && connectionTimer->isActive()) {
            connectionTimer->stop();
    }

    if (progressDialog) {
        QTimer::singleShot(3000, progressDialog, &QProgressDialog::close);
        QTimer::singleShot(3100, this, &MainWindow::showSuccessMessage);
    }
}

void MainWindow::onMqttDisconnected() {
    std::cout << "MainWindow: Received disconnected signal from MqttHandler." << std::endl;

    if (isShowingDialog) {
            return; // 이미 대화상자가 떠 있다면 아무것도 하지 않고 함수 종료
    }
    if (connectionTimer && connectionTimer->isActive()) {
            connectionTimer->stop();
    }
    // "연결 중" 대화상자가 있었다면 먼저 닫아줍니다.
    if (progressDialog) {
        progressDialog->close();
    }

    isShowingDialog = true;
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Connection Fail");
    msgBox.setText("Can not connect MQTT Broker.");
    msgBox.setInformativeText("Retry?");
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setStandardButtons(QMessageBox::Retry | QMessageBox::Close); // '다시 시도'와 '닫기' 버튼 추가
    msgBox.setDefaultButton(QMessageBox::Retry); // '다시 시도'를 기본값으로 설정

    int reply = msgBox.exec(); // 사용자가 버튼을 누를 때까지 대기
    isShowingDialog = false;

    // 사용자의 선택에 따라 동작
    if (reply == QMessageBox::Retry) {
        // '다시 시도'를 누르면 연결 함수를 다시 호출
        startMqttConnection();
    } else {
        // '닫기'를 누르면 프로그램 종료
        close();
    }
}

void MainWindow::startMqttConnection() {
    // 기존에 열려있을 수 있는 대화상자 정리
    if (progressDialog) {
        progressDialog->deleteLater();
    }

    progressDialog = new QProgressDialog("MQTT Connecting...", QString(), 0, 0, this);
    progressDialog->setWindowModality(Qt::WindowModal);
    progressDialog->setCancelButton(nullptr);
    progressDialog->setWindowTitle("Connecting..");
    progressDialog->show();

    progressDialog->raise();          // 다른 창들 위로 올립니다.
    progressDialog->activateWindow(); // 창에 포커스를 주어 활성화합니다.

    if (!connectionTimer) {
        connectionTimer = new QTimer(this);
        connect(connectionTimer, &QTimer::timeout, this, &MainWindow::onMqttDisconnected);
    }
    connectionTimer->setSingleShot(true);
    connectionTimer->start(5000);

    mqttHandler->connectToBroker(My_SERVER_IP, SERVER_PORT);

}

void MainWindow::showSuccessMessage()
{
    QMessageBox::information(this, "Connect Success", "You can MQTT");
}

void MainWindow::onOtaButtonClicked()
{
    if (!mqttHandler || !mqttHandler->isConnected()) {
        QMessageBox::warning(this, "연결 오류", "MQTT 브로커에 연결되지 않았습니다.");
        return;
    }

    // 1. 파일 열기 대화상자를 띄워 사용자로부터 펌웨어 파일을 선택받습니다.
    QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("Select"),
        "", // 시작 디렉토리
        tr("Firmware(*.bin *.hex);;All Files (*.*)") // 파일 필터
    );

    // 2. 사용자가 파일을 선택했는지 확인합니다. (취소 버튼을 누르면 filePath는 비어있음)
    if (filePath.isEmpty()) {
        return; // 아무 파일도 선택하지 않았으므로 함수 종료
    }

    // 3. MqttHandler의 sendFile 함수는 std::string을 인자로 받으므로 QString을 변환합니다.
    std::string stdFilePath = filePath.toStdString();

    // 4. MQTT를 통해 파일 전송을 시작합니다. 토픽은 OTA용으로 지정하는 것이 좋습니다.
    mqttHandler->sendFile("ota/firmware", stdFilePath);

    // 사용자에게 전송 시작을 알리는 메시지 박스 (선택 사항)
    QMessageBox::information(this, "TP start", "STARTING.\n" + filePath);
}

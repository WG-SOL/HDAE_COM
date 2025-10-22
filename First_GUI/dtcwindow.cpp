#include "dtcwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QMessageBox>
#include <QDebug>
#include "My_Mqtt.h"

DTCWindow::DTCWindow(MqttHandler *mqttHandler, QWidget *parent)
    : QWidget(parent)
    , m_mqttHandler(mqttHandler)
{
    setupUi();

    // MqttHandler의 messageReceived 시그널을 updateDtcTable 슬롯에 연결
    if (m_mqttHandler) {
        connect(m_mqttHandler, &MqttHandler::messageReceived, this, &DTCWindow::updateDtcTable);
    }
}

DTCWindow::~DTCWindow()
{
}

void DTCWindow::setupUi()
{
    setWindowTitle("Diagnostic Trouble Codes (DTC)");
    this->resize(640, 480);
    this->setObjectName("DTCWindow"); // QSS 스타일 적용을 위해 이름 설정

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // --- 테이블 위젯 설정 ---
    dtcTable = new QTableWidget(this);
    dtcTable->setColumnCount(2);
    dtcTable->setHorizontalHeaderLabels({"DTC Code", "Description"});
    dtcTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
    dtcTable->setColumnWidth(0, 180);
    dtcTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    dtcTable->verticalHeader()->setVisible(false);
    dtcTable->setEditTriggers(QAbstractItemView::NoEditTriggers); // 편집 불가
    mainLayout->addWidget(dtcTable);

    // --- 버튼 레이아웃 설정 ---
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    readDtcButton = new QPushButton("Read DTC");
    clearTableButton = new QPushButton("Clear Table");

    buttonLayout->addStretch();
    buttonLayout->addWidget(readDtcButton);
    buttonLayout->addWidget(clearTableButton);
    mainLayout->addLayout(buttonLayout);

    // --- 시그널-슬롯 연결 ---
    connect(readDtcButton, &QPushButton::clicked, this, &DTCWindow::onReadDtcsClicked);
    connect(clearTableButton, &QPushButton::clicked, this, &DTCWindow::onClearTableClicked);
}

/**
 * @brief "Read DTCs" 버튼을 누르면 UDS(19 02 FF) 요청을 MQTT로 보냅니다.
 */
void DTCWindow::onReadDtcsClicked()
{
    if (!m_mqttHandler || !m_mqttHandler->isConnected()) {
        QMessageBox::warning(this, "Connection Error", "MQTT client is not connected.");
        return;
    }

    // 1. 테이블을 먼저 비웁니다.
    dtcTable->setRowCount(0);

    // 2. UDS Read DTC Information (19 02 FF) 메시지를 생성합니다.
    // DoIP Header (8 bytes) + DoIP Payload (주소 4 bytes + UDS 3 bytes = 7 bytes)
    const QString hexMessage = "03FC8001000000070E8002011902FF";

    // 3. 16진수 문자열을 바이트 데이터로 변환하여 전송합니다.
    QByteArray payloadBytes = QByteArray::fromHex(hexMessage.toLatin1());
    m_mqttHandler->publishMessage("request/topic", payloadBytes.toStdString());

    qDebug() << "Sent Read DTCs command:" << hexMessage.toUpper();
}

/**
 * @brief 테이블의 모든 내용을 지웁니다.
 */
void DTCWindow::onClearTableClicked()
{
    dtcTable->setRowCount(0);
}

/**
 * @brief [로직 변경] MQTT로 DTC 응답 메시지를 수신했을 때 호출되는 슬롯
 */
void DTCWindow::updateDtcTable(const QString& topic, const QByteArray& payload)
{
    if (topic != "response/topic") return;

    // 최소 길이 및 UDS SID(0x59) 확인
    if (payload.size() < 16 || static_cast<quint8>(payload.at(12)) != 0x59) {
        qWarning() << "Received invalid or non-DTC payload:" << payload.toHex().toUpper();
        return;
    }

    int udsStart = 12;
    QByteArray uds = payload.mid(udsStart);

    // DTC 데이터는 59 02 FF FF 다음부터 시작 (UDS 데이터 기준 4번째 인덱스)
    int dtcStart = 4;
    int remaining = uds.size() - dtcStart;

    if (remaining <= 0) {
        qDebug() << "No DTCs present.";
        return;
    }

    // (DTC 3바이트 + 상태 1바이트) = 4바이트 단위
    if (remaining % 4 != 0) {
        qWarning() << "DTC length invalid: not multiple of 4. UDS data:" << uds.toHex().toUpper();
        return;
    }

    dtcTable->setRowCount(0);

    for (int i = 0; i < remaining; i += 4) {
        QByteArray dtcBytes = uds.mid(dtcStart + i, 3); // DTC 3바이트 추출
        QString hexDtc;

        // [변경] 첫 바이트가 0x01이면 'A'로 치환하는 로직
        if (!dtcBytes.isEmpty() && static_cast<quint8>(dtcBytes.at(0)) == 0x01) {
            // 첫 바이트(01)를 제외한 나머지 2바이트를 16진수 문자열로 변환하고 앞에 'A'를 붙입니다.
            hexDtc = "A" + QString(dtcBytes.mid(1, 2).toHex().toUpper());
        } else {
            // 그 외의 경우는 기존처럼 "0x" 접두사를 붙입니다.
            hexDtc = "0x" + QString(dtcBytes.toHex().toUpper());
        }

        QString description = "Description for " + hexDtc;

        int row = dtcTable->rowCount();
        dtcTable->insertRow(row);
        dtcTable->setItem(row, 0, new QTableWidgetItem(hexDtc));
        dtcTable->setItem(row, 1, new QTableWidgetItem(description));
    }

    qDebug() << "Parsed" << remaining / 4 << "DTC(s).";
}

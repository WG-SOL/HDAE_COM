#include "sensordatawindow.h"
#include "Did_Type.h" // C언어로 정의된 DID 테이블 헤더
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QMessageBox>
#include <QDebug>
#include <QSet>
#include <QDateTime>

SensorDataWindow::SensorDataWindow(MqttHandler *mqttHandler, QWidget *parent)
    : QWidget(parent)
    , m_mqttHandler(mqttHandler)
    , m_isLiveMode(false)
{
    setupUi();
    populateTable();

    // [변경] Did_Type.cpp의 didTable을 기반으로 양방향 맵을 동적으로 생성합니다.
    for (int i = 0; i < DID_TABLE_SIZE; ++i) {
        QString fullDescription = QString::fromLatin1(didTable[i].description);
        QString sensorName = fullDescription.section('_', 0, 0); // "ToF_..." -> "ToF"
        quint16 did = didTable[i].did;

        m_didToNameMap[did] = sensorName;   // DID -> 이름
        m_nameToDidMap[sensorName] = did;   // 이름 -> DID
    }

    if (m_mqttHandler) {
        connect(m_mqttHandler, &MqttHandler::messageReceived, this, &SensorDataWindow::updateSensorData);
    }

    m_liveUpdateTimer = new QTimer(this);
    connect(m_liveUpdateTimer, &QTimer::timeout, this, &SensorDataWindow::requestLiveUpdates);
}

SensorDataWindow::~SensorDataWindow()
{
}

// [변경 없음] setupUi() 함수는 이전과 동일합니다.
void SensorDataWindow::setupUi()
{
    setWindowTitle("Sensor Data Dashboard");
    this->resize(800, 600);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    sensorTable = new QTableWidget(this);
    mainLayout->addWidget(sensorTable);

    sensorTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    sensorTable->setSelectionMode(QAbstractItemView::MultiSelection);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    updateButton = new QPushButton("Update");
    liveButton = new QPushButton("Live");
    clearButton = new QPushButton("Clear");
    allButton = new QPushButton("All");

    buttonLayout->addStretch();
    buttonLayout->addWidget(updateButton);
    buttonLayout->addWidget(liveButton);
    buttonLayout->addWidget(clearButton);
    buttonLayout->addWidget(allButton);
    buttonLayout->addStretch();
    mainLayout->addLayout(buttonLayout);

    connect(updateButton, &QPushButton::clicked, this, &SensorDataWindow::onUpdateButtonClicked);
    connect(liveButton, &QPushButton::clicked, this, &SensorDataWindow::onLiveButtonClicked);
    connect(clearButton, &QPushButton::clicked, this, &SensorDataWindow::onClearButtonClicked);
    connect(allButton, &QPushButton::clicked, this, &SensorDataWindow::onAllButtonClicked);
}

// [변경 없음] populateTable() 함수는 이전과 동일합니다.
void SensorDataWindow::populateTable()
{
    sensorTable->setColumnCount(3);
    sensorTable->setHorizontalHeaderLabels({"Name", "Value", "State"});
    sensorTable->verticalHeader()->setVisible(false);
    sensorTable->verticalHeader()->setDefaultSectionSize(40);
    sensorTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    sensorTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    sensorTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);

    const QStringList sensorNames = {
        "ToF", "LightSensor", "ABEThreshold", "HeadlightThreshold"
    };
    sensorTable->setRowCount(sensorNames.count());

    for (int i = 0; i < sensorNames.count(); ++i) {
        QTableWidgetItem *nameItem = new QTableWidgetItem(sensorNames.at(i));
        nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
        sensorTable->setItem(i, 0, nameItem);

        QTableWidgetItem *valueItem = new QTableWidgetItem("");
        valueItem->setFlags(valueItem->flags() & ~Qt::ItemIsEditable);
        sensorTable->setItem(i, 1, valueItem);

        QTableWidgetItem *stateItem = new QTableWidgetItem("");
        stateItem->setFlags(stateItem->flags() & ~Qt::ItemIsEditable);
        sensorTable->setItem(i, 2, stateItem);
    }
}

// [변경 없음] onAllButtonClicked() 함수는 이전과 동일합니다.
void SensorDataWindow::onAllButtonClicked()
{
    if (sensorTable->selectedItems().count() == sensorTable->rowCount() * sensorTable->columnCount()) {
        sensorTable->clearSelection();
    } else {
        sensorTable->selectAll();
    }
}


/**
 * @brief [로직 변경] Update 버튼 클릭 시 UDS 형식의 MQTT 메시지를 생성하여 전송합니다.
 */
void SensorDataWindow::onUpdateButtonClicked()
{
    if (!m_mqttHandler || !m_mqttHandler->isConnected()) {
        QMessageBox::warning(this, "Connection Error", "MQTT client is not connected.");
        return;
    }
    QModelIndexList selectedRows = sensorTable->selectionModel()->selectedRows();
    if (selectedRows.isEmpty()) {
        QMessageBox::information(this, "No Selection", "Please select one or more sensors to update.");
        return;
    }

    QSet<int> uniqueRows;
    for(const QModelIndex &index : selectedRows) {
        uniqueRows.insert(index.row());
    }

    for (int row : uniqueRows) {
        QString sensorName = sensorTable->item(row, 0)->text();
        publishUdsRequest(sensorName); // 헬퍼 함수 호출
        sensorTable->item(row, 2)->setText("Requesting...");
    }
}


void SensorDataWindow::onLiveButtonClicked()
{
    if (!m_mqttHandler || !m_mqttHandler->isConnected()) {
        QMessageBox::warning(this, "Connection Error", "MQTT client is not connected.");
        return;
    }
    QModelIndexList selectedRows = sensorTable->selectionModel()->selectedRows();
    if (selectedRows.isEmpty()) {
        QMessageBox::information(this, "No Selection", "Please select sensors for live updates.");
        return;
    }
    onClearButtonClicked();
    m_isLiveMode = true;
    QSet<int> uniqueRows;
    for(const QModelIndex &index : selectedRows) {
        uniqueRows.insert(index.row());
    }
    for(int row : uniqueRows) {
        QString sensorName = sensorTable->item(row, 0)->text();
        m_liveSensorNames.append(sensorName);
        sensorTable->item(row, 2)->setText("Live 🟢");
    }
    if (!m_liveUpdateTimer->isActive()) {
        m_liveUpdateTimer->start(1000);
    }
    requestLiveUpdates();
}

void SensorDataWindow::onClearButtonClicked()
{
    m_liveUpdateTimer->stop();
    m_isLiveMode = false;
    m_liveSensorNames.clear();
    for (int i = 0; i < sensorTable->rowCount(); ++i) {
        sensorTable->item(i, 1)->setText("");
        sensorTable->item(i, 2)->setText("");
    }
    qDebug() << "Live mode stopped and table cleared.";
}
void SensorDataWindow::requestLiveUpdates()
{
    if (!m_isLiveMode || m_liveSensorNames.isEmpty()) {
        m_liveUpdateTimer->stop();
        return;
    }
    if (!m_mqttHandler || !m_mqttHandler->isConnected()) {
        QMessageBox::warning(this, "Connection Error", "MQTT client disconnected. Stopping live updates.");
        onClearButtonClicked();
        return;
    }

    // Live 목록에 있는 모든 센서에 대해 UDS 요청을 보냅니다.
    for (const QString &sensorName : m_liveSensorNames) {
        publishUdsRequest(sensorName); // 헬퍼 함수 호출
    }
}
void SensorDataWindow::updateSensorData(const QString& topic, const QByteArray& payload)
{
    if (topic != "response/topic") return;

    if (payload.size() < 17 || static_cast<quint8>(payload.at(12)) != 0x62) {
        qWarning() << "Received invalid or non-UDS payload:" << payload.toHex().toUpper();
        return;
    }

    quint8 did_high = payload.at(13);
    quint8 did_low = payload.at(14);
    quint16 did = (static_cast<quint16>(did_high) << 8) | did_low;

    if (!m_didToNameMap.contains(did)) {
        qWarning() << "Received data for unknown DID:" << QString::number(did, 16).toUpper();
        return;
    }
    QString sensorName = m_didToNameMap[did];

    quint8 val_high = payload.at(15);
    quint8 val_low = payload.at(16);
    quint16 value = (static_cast<quint16>(val_high) << 8) | val_low;
    QString valueStr = QString::number(value);

    QList<QTableWidgetItem *> items = sensorTable->findItems(sensorName, Qt::MatchExactly);
    if (items.isEmpty()) return;

    int row = items.first()->row();

    sensorTable->item(row, 1)->setText(valueStr);

    if (!m_liveSensorNames.contains(sensorName)) {
        sensorTable->item(row, 2)->setText(QDateTime::currentDateTime().toString("hh:mm:ss"));
    }
}

void SensorDataWindow::publishUdsRequest(const QString &sensorName)
{
    // 1. 센서 이름으로 DID를 조회합니다.
    if (!m_nameToDidMap.contains(sensorName)) {
        qWarning() << "Could not find DID for sensor name:" << sensorName;
        return;
    }
    quint16 did = m_nameToDidMap[sensorName];

    // 2. UDS 헤더와 DID를 조합하여 16진수 문자열 메시지를 만듭니다.
    const QString udsHeader = "03FC8001000000070E80020122";
    QString didString = QString("%1").arg(did, 4, 16, QChar('0')).toUpper();
    QString hexMessage = udsHeader + didString;

    // 3. 16진수 문자열을 실제 바이트 데이터(QByteArray)로 변환합니다.
    QByteArray payloadBytes = QByteArray::fromHex(hexMessage.toLatin1());

    // 4. 변환된 바이트 데이터를 MQTT로 전송합니다.
    // 참고: MqttHandler의 publishMessage가 std::string을 받도록 되어있으므로 변환합니다.
    m_mqttHandler->publishMessage("request/topic", payloadBytes.toStdString());
}

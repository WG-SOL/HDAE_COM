#include "modifywindow.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QDebug>
#include <QIntValidator>

ModifyWindow::ModifyWindow(MqttHandler* handler, QWidget* parent)
    : QWidget(parent), m_mqttHandler(handler)
{
    this->setWindowTitle("Modify Data by DID");
    this->resize(450, 250);
    this->setObjectName("ModifyWindow"); // QSS 스타일 적용을 위해 이름 설정

    this->setStyleSheet(R"(
            #ModifyWindow {
                background-color: #2c3e50;
            }
            #ModifyWindow QLabel {
                color: #ecf0f1;
                font-weight: bold;
                margin-top: 5px;
            }
            #ModifyWindow QLineEdit {
                background-color: #2c3e50;
                border: 1px solid #555;
                border-radius: 4px;
                padding: 8px;
                color: #ecf0f1;
                font-size: 12pt;
            }
            #ModifyWindow QLineEdit:focus {
                border: 1px solid #3498db; /* 입력 시 파란색 테두리로 강조 */
            }
        )");


    QLabel* abeLabel = new QLabel("ABE Threshold (DID: 0x2001):", this);
    abeEdit = new QLineEdit(this);
    // 입력 값 검증을 위한 Validator 설정 (0~65535 사이의 숫자만 입력 가능)
    abeEdit->setValidator(new QIntValidator(0, 65535, this));
    abeEdit->setPlaceholderText("Enter a value between 0 and 65535");

    QLabel* headLabel = new QLabel("Headlight Threshold (DID: 0x2002):", this);
    headlightEdit = new QLineEdit(this);
    headlightEdit->setValidator(new QIntValidator(0, 65535, this));
    headlightEdit->setPlaceholderText("Enter a value between 0 and 65535");

    sendButton = new QPushButton("Send Write Command(s)", this);
    connect(sendButton, &QPushButton::clicked, this, &ModifyWindow::onSendClicked);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(10);
    layout->setContentsMargins(20, 15, 20, 15); // 전체 여백 조정
    layout->addWidget(abeLabel);
    layout->addWidget(abeEdit);
    layout->addSpacing(10);
    layout->addWidget(headLabel);
    layout->addWidget(headlightEdit);
    layout->addStretch();
    layout->addWidget(sendButton);
    setLayout(layout);
}

/**
 * @brief UDS Write(0x2E) 프레임을 생성합니다. SID + DID(2) + Value(2) = 5 bytes
 */
QByteArray ModifyWindow::buildUdsWriteFrame(quint16 did, quint16 value)
{
    QByteArray frame;
    frame.append(0x2E); // SID: WriteDataByIdentifier

    // DID (2바이트, Big-Endian)
    frame.append(static_cast<char>((did >> 8) & 0xFF));
    frame.append(static_cast<char>(did & 0xFF));

    // Data (2바이트, Big-Endian)
    frame.append(static_cast<char>((value >> 8) & 0xFF));
    frame.append(static_cast<char>(value & 0xFF));

    return frame;
}

/**
 * @brief [핵심 수정] DoIP 헤더와 UDS 프레임을 결합하여 올바른 길이로 MQTT 메시지를 전송합니다.
 */
void ModifyWindow::sendWriteRequest(quint16 did, quint16 value)
{
    // 1. UDS 프레임을 먼저 생성합니다 (5 바이트).
    QByteArray udsFrame = buildUdsWriteFrame(did, value);

    // 2. DoIP 페이로드를 구성합니다. (논리 주소 4바이트 + UDS 프레임)
    QByteArray doipPayload;
    doipPayload.append(QByteArray::fromHex("0E800201")); // Source & Target Address
    doipPayload.append(udsFrame);

    // 3. DoIP 페이로드의 실제 길이를 계산합니다.
    quint32 payloadLength = doipPayload.size();

    // 4. DoIP 헤더를 생성하고, 계산된 길이를 Big-Endian으로 삽입합니다.
    QByteArray doipHeader;
    doipHeader.append(QByteArray::fromHex("03FC8001")); // Protocol Version, etc.
    doipHeader.append(static_cast<char>((payloadLength >> 24) & 0xFF));
    doipHeader.append(static_cast<char>((payloadLength >> 16) & 0xFF));
    doipHeader.append(static_cast<char>((payloadLength >> 8) & 0xFF));
    doipHeader.append(static_cast<char>(payloadLength & 0xFF));

    // 5. 헤더와 페이로드를 합쳐 최종 메시지를 완성합니다.
    QByteArray finalMessage = doipHeader + doipPayload;

    // 6. MQTT로 전송합니다.
    m_mqttHandler->publishMessage("request/topic", finalMessage.toStdString());
    qDebug() << "[MQTT] Sent WriteDID Request. DID:" << QString::number(did, 16)
             << "Value:" << value << "Payload:" << finalMessage.toHex().toUpper();
}


void ModifyWindow::onSendClicked()
{
    if (!m_mqttHandler || !m_mqttHandler->isConnected()) {
        QMessageBox::warning(this, "MQTT Error", "Broker not connected.");
        return;
    }

    bool abeOk = false;
    quint16 abeVal = abeEdit->text().toUShort(&abeOk);

    bool headOk = false;
    quint16 headVal = headlightEdit->text().toUShort(&headOk);

    if (!abeOk && !headOk) { // 둘 다 입력이 없거나 유효하지 않은 경우
        QMessageBox::warning(this, "Input Error", "Please enter at least one valid numeric value.");
        return;
    }

    if (abeOk) {
        sendWriteRequest(0x2001, abeVal);
    }

    if (headOk) {
        sendWriteRequest(0x2002, headVal);
    }

    QMessageBox::information(this, "Success", "Write command(s) sent successfully.");
}

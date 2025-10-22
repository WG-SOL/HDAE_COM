#ifndef DTCWINDOW_H
#define DTCWINDOW_H

#include <QWidget>
#include "My_Mqtt.h" // MqttHandler

// Qt 클래스 전방 선언
class QTableWidget;
class QPushButton;

/**
 * @class DTCWindow
 * @brief DTC(고장 코드)를 요청하고 결과를 테이블에 표시하는 창 클래스입니다.
 */
class DTCWindow : public QWidget
{
    Q_OBJECT

public:
    explicit DTCWindow(MqttHandler *mqttHandler, QWidget *parent = nullptr);
    ~DTCWindow();

private slots:
    /**
     * @brief "Read DTCs" 버튼 클릭 시 UDS 요청을 보냅니다.
     */
    void onReadDtcsClicked();

    /**
     * @brief "Clear Table" 버튼 클릭 시 테이블 내용을 지웁니다.
     */
    void onClearTableClicked();

    /**
     * @brief MQTT로 DTC 응답을 받았을 때 테이블을 업데이트합니다.
     */
    void updateDtcTable(const QString& topic, const QByteArray& payload);

private:
    void setupUi();

    /**
     * @brief 3바이트 DTC 데이터를 "P0123"과 같은 표준 형식의 문자열로 변환합니다.
     */
    QString formatDtc(const QByteArray& dtcBytes);

    // --- 멤버 변수 ---
    MqttHandler *m_mqttHandler;
    QTableWidget *dtcTable;
    QPushButton *readDtcButton;
    QPushButton *clearTableButton;
};

#endif // DTCWINDOW_H

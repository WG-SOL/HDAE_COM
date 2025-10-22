#ifndef SENSORDATAWINDOW_H
#define SENSORDATAWINDOW_H

#include <QWidget>
#include <QTimer>
#include <QString>
#include <QByteArray>
#include <QMap>
#include "My_Mqtt.h"

// Qt 클래스들의 전방 선언
class QTableWidget;
class QPushButton;

class SensorDataWindow : public QWidget
{
    Q_OBJECT

public:
    explicit SensorDataWindow(MqttHandler *mqttHandler, QWidget *parent = nullptr);
    ~SensorDataWindow();

private slots:
    void onUpdateButtonClicked();
    void onLiveButtonClicked();
    void onClearButtonClicked();
    void onAllButtonClicked();
    void requestLiveUpdates();
    void updateSensorData(const QString& topic, const QByteArray& payload);

private:
    void setupUi();
    void populateTable();
    void publishUdsRequest(const QString& sensorName);


    // --- 멤버 변수 ---
    MqttHandler *m_mqttHandler;
    QTableWidget *sensorTable;
    QPushButton *updateButton;
    QPushButton *liveButton;
    QPushButton *clearButton;
    QPushButton *allButton;
    QTimer *m_liveUpdateTimer;
    bool m_isLiveMode;
    QStringList m_liveSensorNames;

    // DID <-> 센서 이름 양방향 매핑을 위한 맵
    QMap<quint16, QString> m_didToNameMap;     // DID로 이름 찾기 (응답 파싱용)
    QMap<QString, quint16> m_nameToDidMap;     // [추가] 이름으로 DID 찾기 (요청 생성용)
};

#endif // SENSORDATAWINDOW_H


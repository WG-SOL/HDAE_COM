#ifndef SCANSELECTIONWINDOW_H
#define SCANSELECTIONWINDOW_H

#include <QWidget>
#include "My_Mqtt.h"

class DTCWindow;
class SensorDataWindow;
class ActuationTestWindow;
class QPushButton;
class ModifyWindow;

class ScanSelectionWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ScanSelectionWindow(MqttHandler *mqttHandler, QWidget *parent = nullptr);
    ~ScanSelectionWindow();

private slots:
    void openDTCWindow();
    void openSensorDataWindow();
    void openActuationTestWindow();
    void onSessionButtonClicked();
    void openModifyWindow();

private:
    void setupUi();
    void updateButtonStates();

    QPushButton *dtcButton;
    QPushButton *sensorDataButton;
    QPushButton *actuationButton;
    QPushButton *modifyButton;
    QPushButton *sessionButton;


    DTCWindow *m_dtcWindow;
    SensorDataWindow *m_sensorDataWindow;
    ActuationTestWindow *m_actuationTestWindow;
    ModifyWindow *m_modifyWindow;

    MqttHandler *m_mqttHandler;

    bool m_isExtendedSession;
};

#endif // SCANSELECTIONWINDOW_H

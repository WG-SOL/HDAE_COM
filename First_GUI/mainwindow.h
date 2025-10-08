#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QToolButton>
#include <My_Mqtt.h>
#include <QProgressDialog>
#include <QTimer>
// DTC
class DTCWindow;

class MainWindow : public QMainWindow
{
    Q_OBJECT

    public:
        MainWindow(QWidget *parent = nullptr);
        ~MainWindow();

    protected:
        void resizeEvent(QResizeEvent *event) override;

    private slots:
        void openDTCWindow(); // DTC
        void onMqttConnected();
        void onMqttDisconnected();
        void showSuccessMessage();
        void onOtaButtonClicked(); // *** 추가된 부분 ***: OTA 버튼 클릭을 위한 슬롯

    private:
        QToolButton *scanTechButton; //
        QToolButton *OTAButton; // 멤버 변수로 변경
        DTCWindow *dtcWindow;
        QWidget *overlay;
        MqttHandler *mqttHandler;
        QProgressDialog *progressDialog;
        void startMqttConnection();
        QTimer *connectionTimer;
        bool isShowingDialog;
};
#endif // MAINWINDOW_H

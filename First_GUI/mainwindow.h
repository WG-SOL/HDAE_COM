#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QToolButton>
#include <My_Mqtt.h>
#include <QProgressDialog>
#include <QTimer>

// ★★★ 1. 오타 수정 및 올바른 헤더 포함 (ScanSelectionWindow.h) ★★★
#include "ScanSelectionWindow.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void on_scanButton_clicked();
    void onMqttConnected();
    void onMqttDisconnected();
    void showSuccessMessage();
    void onOtaButtonClicked();

private:
    QToolButton *scanTechButton;
    QToolButton *OTAButton;
    QToolButton *savedDataButton;
    // ★★★ 2. 멤버 변수의 타입을 헤더 파일의 클래스 이름과 일치시킴 ★★★
    ScanSelectionWindow *m_selectionWindow;
    QWidget *overlay;
    MqttHandler *mqttHandler;
    QProgressDialog *progressDialog;
    void startMqttConnection();
    QTimer *connectionTimer;
    bool isShowingDialog;
};
#endif // MAINWINDOW_H

#ifndef DTCWINDOW_H
#define DTCWINDOW_H

#include <QWidget>
#include <QTableWidget>

class DTCWindow : public QWidget
{
    Q_OBJECT

    public:
        explicit DTCWindow(QWidget *parent = nullptr);
        ~DTCWindow();

    private slots:
        void onClearButtonClicked();
    private:
        QTableWidget *dtcTable;
        void setupUi();
        void populateTable(); // 테이블에 데이터를 채우는 함수
};

#endif // DTCWINDOW_H

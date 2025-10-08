#include "mainwindow.h"
#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    // G-scan 2 테마 QSS 스타일 문자열
    QString qss = R"(
       /* 메인 윈도우 배경 이미지 */
        QMainWindow#MainWindow {
            border-image: url(:/images/Autoever.jpg);
        }

        /* DTCWindow 등 새로 뜨는 창들의 배경 */
        DTCWindow, QDialog {
            background-color: #2c3e50;
        }

        /* 모든 위젯의 기본 폰트 색상 및 크기 */
        QWidget {
            color: #ecf0f1;
            font-size: 12pt;
        }

        /* ==================== 2. 개별 위젯 스타일 ==================== */



        /* --- 툴 버튼 --- */
        QToolButton, QPushButton {
            color: white;
            border: 1px solid #222;
            border-radius: 8px;
            padding: 10px;
            font-weight: bold;

        }
        QToolButton:hover, QPushButton:hover {
            border: 1px solid #777;
        }
        QToolButton:pressed, QPushButton:pressed {
            background-color: #2c3a4a;
            border: 1px solid #111;
        }


        /*=============== Each Button ===================*/
        /* ★★★ 버튼 ID별로 다른 배경색(그라데이션) 지정 ★★★ */
        QToolButton#savedDataButton {
            background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #f39c12, stop:1 #d35400); /* 주황색 */
        }
        QToolButton#OTAButton {
            background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #9b59b6, stop:1 #8e44ad); /* 보라색 */
        }
        QToolButton#scanTechButton {
            background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #3498db, stop:1 #2980b9); /* 파란색 */
        }


        /* --- 테이블 위젯 --- */
        QTableWidget {
            background-color: #34495e;
            border: 1px solid #444;
            gridline-color: #444;
        }
        /* 테이블 헤더 */
        QHeaderView::section {
            background-color: #5c6a7a;
            padding: 5px;
            border: none; /* 헤더 테두리 단순화 */
            font-weight: bold;
        }
        /* 테이블 아이템 선택 시 */
        QTableWidget::item:selected {
            background-color: #3498db;
        }
    )";

    // 애플리케이션 전체에 QSS 적용
    a.setStyleSheet(qss);

    MainWindow w;
    w.show();
    return a.exec();
}

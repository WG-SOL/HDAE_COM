#include "mainwindow.h"
#include <QApplication>


int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    // G-scan 2 테마 QSS 스타일 문자열
    QString qss = R"(
      /* =============  Back-ground Setting ================ */

      QMainWindow#MainWindow {
          border-image: url(:/images/Autoever.jpg);
      }

      #backgroundContainer {
          background-image: url(:/images/GIT_Background.jpg);
      }

      /* [변경] 새로 뜨는 모든 창들의 배경 (ActuationTestWindow 추가) */
      DTCWindow, SensorDataWindow, ActuationTestWindow, QDialog {
          background-color: #2c3e50;
      }

      QWidget {
         color: #ecf0f1;
         font-size: 12pt;
      }





        /* ==================== 2. 개별 위젯 스타일 ==================== */



        /* --- 툴 버튼 --- */
        QToolButton, QPushButton {
          color: white;
          background-color: #34495e;  /* ← 이 줄 추가 */
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

        /* --- [추가] 그룹박스, 콤보박스, 스핀박스, 라벨 스타일 --- */
          QLabel {
              color: #ecf0f1; /* 라벨 글자색을 밝게 설정 */
              background-color: transparent; /* 라벨 배경 투명하게 */
          }
          QGroupBox {
              background-color: #34495e;
              border: 1px solid #444;
              border-radius: 8px;
              margin-top: 10px;
              font-weight: bold;
          }
          QGroupBox::title {
              subcontrol-origin: margin;
              subcontrol-position: top center;
              padding: 0 10px;
              color: #ecf0f1;
          }
          QComboBox, QSpinBox {
              background-color: #2c3e50;
              border: 1px solid #555;
              border-radius: 4px;
              padding: 5px;
              color: #ecf0f1;
          }
          QComboBox::drop-down {
              border: none;
          }
          QComboBox::down-arrow {
              image: url(:/qt-project.org/styles/commonstyle/images/down-arrow-light.png);
          }
          QComboBox QAbstractItemView {
                      background-color: #34495e; /* 드롭다운 배경색 */
                      border: 1px solid #555;
                      selection-background-color: #3498db; /* 선택 항목 배경색 */
                      color: #ecf0f1; /* 드롭다운 글자색 */
                      outline: 0px; /* 포커스 테두리 제거 */
           }
          QSpinBox::up-button, QSpinBox::down-button {
              border: none;
              background-color: #5c6a7a;
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


        /*=============== Scan Select =====================*/
        #ScanSelectionWindow #headerContainer {
          background-color: #141d2b; /* 짙은 남색 배경 */
          min-height: 50px; /* 헤더의 최소 높이 지정 */
        }
        /* 헤더 안의 "Scan" 텍스트 라벨 */
        #ScanSelectionWindow #headerContainer QLabel {
          color: white;
          font: bold 16pt;
          background-color: transparent;
        }
        /* 헤더 안의 세션 버튼 */
        #ScanSelectionWindow #sessionButton {
          font-size: 11pt;      /* 폰트 크기 축소 */
          padding: 5px 15px;    /* 내부 여백 조정 */
          border-radius: 15px;  /* 모서리를 더 둥글게 */
          max-height: 30px;     /* 최대 높이를 헤더보다 작게 제한 */
        }

          /* 3. 버튼 스타일 (테두리 없음, 투명 배경, 하단 구분선) */
          #ScanSelectionWindow QPushButton {
              background-color: transparent;
              border: none;
              border-bottom: 2px solid rgba(255, 255, 255, 0.3);
              color: white;
              font: bold 16pt;
              text-align: left;
              padding: 20px;
              padding-left: 25px;
              border-radius: 0px; /* 직사각형 버튼 */
          }

          #ScanSelectionWindow QPushButton:first-child {
              border-top: 2px solid rgba(255, 255, 255, 0.3);
          }

          #ScanSelectionWindow QPushButton:hover {
              background-color: rgba(255, 255, 255, 0.1);
          }

          #ScanSelectionWindow QPushButton:pressed {
              background-color: rgba(0, 0, 0, 0.1);
          }

          #ScanSelectionWindow QPushButton:disabled {
              color: #999;
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

        QPushButton#sessionButton[sessionState="normal"] {S
            background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #3498db, stop:1 #2980b9); /* 파란색 그라데이션 */
        }
        QPushButton#sessionButton[sessionState="extended"] {
            background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #2ecc71, stop:1 #27ae60); /* 녹색 그라데이션 */
        }
    )";

    // 애플리케이션 전체에 QSS 적용
    a.setStyleSheet(qss);

    MainWindow w;
    w.show();

    return a.exec();
}


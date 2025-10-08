#include "dtcwindow.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QHBoxLayout>
#include <QHeaderView>
DTCWindow::DTCWindow(QWidget *parent) : QWidget(parent)
{
    setupUi();
    populateTable();
}

DTCWindow::~DTCWindow()
{
}

void DTCWindow::setupUi()
{
    setWindowTitle("Error Code");
    this->resize(640, 480);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    dtcTable = new QTableWidget(this);
    mainLayout->addWidget(dtcTable);

    // 하단 액션 버튼 추가
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *stateButton = new QPushButton("State");
    QPushButton *clearButton = new QPushButton("Clear");
    QPushButton *rescanButton = new QPushButton("ReScan");

    buttonLayout->addWidget(stateButton);
    buttonLayout->addWidget(clearButton);
    buttonLayout->addWidget(rescanButton);
    mainLayout->addLayout(buttonLayout);

    connect(clearButton, &QPushButton::clicked, this, &DTCWindow::onClearButtonClicked);
}

void DTCWindow::onClearButtonClicked()
{
    // QTableWidget의 모든 행을 삭제하는 가장 간단하고 효율적인 방법입니다.
    dtcTable->setRowCount(0);
}

void DTCWindow::populateTable()
{
    // 테이블 설정
    dtcTable->setColumnCount(3);
    dtcTable->setHorizontalHeaderLabels({"ErrorCode", "Name", "State"});

    // ★★★ 왼쪽의 불필요한 행 번호 헤더를 숨깁니다. ★★★
    dtcTable->verticalHeader()->setVisible(false);

    // 예시 데이터 추가
    QStringList dtcCodes = {"P0011", "P0016", "P0031"};
    QStringList dtcNames = {"'A' shaft", "Shaft which Error", "Oxygen Low"};
    QStringList dtcStates = {"Now", "Prev", "Now"};

    for (int i = 0; i < dtcCodes.size(); ++i) {
        int row = dtcTable->rowCount();
        dtcTable->insertRow(row);
        dtcTable->setItem(row, 0, new QTableWidgetItem(dtcCodes[i]));
        dtcTable->setItem(row, 1, new QTableWidgetItem(dtcNames[i]));
        dtcTable->setItem(row, 2, new QTableWidgetItem(dtcStates[i]));
    }

    // 컬럼 너비 자동 조절
    dtcTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
}

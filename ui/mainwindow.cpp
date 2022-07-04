#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QDebug>
#include <QPushButton>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    init();
}

MainWindow::~MainWindow()
{
    delete m_pModbusHandler;
    delete ui;
}

void MainWindow::init()
{
    setWindowTitle(tr("NetAssist"));

    ui->ipLineEdit->setInputMask("000.000.000.000:0000;");
    ui->ipLineEdit->setText("127.0.0.1:5020");
    switchConnectMode(false);

    initTableWidget();

    ui->editCheckBox->setChecked(true);
    switchEditMode(true);
    switchEditButtonState(false);

    m_pModbusHandler = new ModbusHandler(this);
    connect(m_pModbusHandler, &ModbusHandler::modbusStateChanged, this, &MainWindow::onModbusStateChanged);

    connect(ui->connectPushButton, &QPushButton::clicked, this, &MainWindow::onConnectButtonClicked);
    connect(ui->readPushButton, &QPushButton::clicked, this, &MainWindow::onReadButtonClicked);
    connect(ui->appendPushButton, &QPushButton::clicked, this, &MainWindow::onAppendRowButtonClicked);
    connect(ui->removePushButton, &QPushButton::clicked, this, &MainWindow::onRemoveRowButtonClicked);
    connect(ui->insertPushButton, &QPushButton::clicked, this, &MainWindow::onInsertRowButtonClicked);
    connect(ui->editCheckBox, &QCheckBox::clicked, this, &MainWindow::onEditCheckBoxClicked);
}

void MainWindow::initTableWidget()
{
    QStringList strList;
    strList << tr("Name") << tr("Value") << tr("Read") << tr("Write")
            << tr("Block") << tr("Adress") << tr("Ratio") << tr("BatchRead") << tr("BatchWrite");
    ui->tableWidget->setColumnCount(strList.count());
    ui->tableWidget->setHorizontalHeaderLabels(strList);
    ui->tableWidget->clearContents();
    ui->tableWidget->setAlternatingRowColors(true);
//    ui->tableWidget->resizeColumnsToContents();

    connect(ui->tableWidget, &QTableWidget::currentCellChanged, this, &MainWindow::onTableCurrentCellChanged);
}

void MainWindow::switchConnectMode(bool isOn)
{
    if (isOn)
    {
        ui->connectPushButton->setText(tr("Disconnect"));
        ui->ipLineEdit->setEnabled(false);
        ui->slaveSpinBox->setEnabled(false);
    }
    else
    {
        ui->connectPushButton->setText(tr("Connect"));
        ui->ipLineEdit->setEnabled(true);
        ui->slaveSpinBox->setEnabled(true);
    }
}

void MainWindow::switchEditMode(bool isOn)
{
    if (isOn)
    {
        ui->editGroupBox->setEnabled(true);
        ui->readWriteGroupBox->setEnabled(false);
        ui->tableWidget->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked);
    }
    else
    {
        ui->editGroupBox->setEnabled(false);
        ui->readWriteGroupBox->setEnabled(true);
        ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    }
}

void MainWindow::switchEditButtonState(bool isOn)
{
    if (isOn)
    {
        if (!ui->insertPushButton->isEnabled())
        {
            ui->insertPushButton->setEnabled(true);
            ui->removePushButton->setEnabled(true);
        }
    }
    else
    {
        if (ui->insertPushButton->isEnabled())
        {
            ui->insertPushButton->setEnabled(false);
            ui->removePushButton->setEnabled(false);
        }
    }
}

void MainWindow::onConnectButtonClicked()
{
    m_pModbusHandler->tryConnect(ui->ipLineEdit->text());
}

void MainWindow::onReadButtonClicked()
{
    m_pModbusHandler->tryRead();
}

void MainWindow::onAppendRowButtonClicked()
{
    ui->tableWidget->insertRow(ui->tableWidget->rowCount());
}

void MainWindow::onRemoveRowButtonClicked()
{
//    qDebug() << "Current row: " << ui->tableWidget->currentRow();
    ui->tableWidget->removeRow(ui->tableWidget->currentRow());
    ui->tableWidget->setCurrentCell(-1, -1);
}

void MainWindow::onInsertRowButtonClicked()
{
//    qDebug() << "Current row: " << ui->tableWidget->currentRow();
    ui->tableWidget->insertRow(ui->tableWidget->currentRow());
//    ui->tableWidget->setCurrentCell(-1, -1);
}

void MainWindow::onTableCurrentCellChanged(int nCurRow, int nCurCol, int nPreRow, int nPreCol)
{
    qDebug() << "current row:" << nCurRow;
    switchEditButtonState(nCurRow != -1);
    //    ui->tableWidget->resizeColumnsToContents();
}

void MainWindow::onEditCheckBoxClicked(bool isOn)
{
    switchEditMode(isOn);
}

void MainWindow::onModbusStateChanged(bool isOn)
{
    switchConnectMode(isOn);
}





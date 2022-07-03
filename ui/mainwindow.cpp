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
//    connect(ui->connectPushButton, &QPushButton::clicked, this, &MainWindow::on_connectButton_clicked);
}

MainWindow::~MainWindow()
{
    delete m_pModbusHandler;
    delete ui;
}

void MainWindow::init()
{
    ui->ipLineEdit->setInputMask("000.000.000.000:0000;");
    ui->ipLineEdit->setText("127.0.0.1:5020");

    m_pModbusHandler = new ModbusHandler(this);
    connect(m_pModbusHandler, &ModbusHandler::modbusStateChanged, this, &MainWindow::onModbusStateChanged);
}

void MainWindow::on_connectPushButton_clicked()
{
    m_pModbusHandler->tryConnect(ui->ipLineEdit->text());
}

void MainWindow::on_readPushButton_clicked()
{
    m_pModbusHandler->tryRead();
}

void MainWindow::onModbusStateChanged(bool isOn)
{
    if (isOn)
    {
        ui->connectPushButton->setText(tr("Disconnect"));
    }
    else
    {
        ui->connectPushButton->setText(tr("Connect"));
    }
}





#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QDebug>
#include <QPushButton>
#include "modbushandler.h"


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
    delete ui;
}

void MainWindow::init()
{
    ui->ipLineEdit->setInputMask("000.000.000.000:0000; ");
    ui->ipLineEdit->setText("127.0.0.1:5020");
}

void MainWindow::on_connectPushButton_clicked()
{
    qDebug()<< "connect";
    auto pModbusHandler = new ModbusHandler();
    pModbusHandler->tryConnect(ui->ipLineEdit->text());
    delete pModbusHandler;
}


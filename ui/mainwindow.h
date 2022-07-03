#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "modbushandler.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_connectPushButton_clicked();
    void on_readPushButton_clicked();
    void onModbusStateChanged(bool isOn);

private:
    Ui::MainWindow *ui;
    ModbusHandler* m_pModbusHandler = nullptr;
    void init();
};
#endif // MAINWINDOW_H

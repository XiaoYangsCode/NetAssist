#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidgetItem>
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
    void onConnectButtonClicked();
    void onReadButtonClicked();
    void onAppendRowButtonClicked();
    void onRemoveRowButtonClicked();
    void onInsertRowButtonClicked();
    void onTableCurrentCellChanged(int nCurRow, int nCurCol, int nPreRow, int nPreCol);
    void onEditCheckBoxClicked(bool isOn);

    void onModbusStateChanged(bool isOn);

private:
    Ui::MainWindow *ui;
    ModbusHandler* m_pModbusHandler = nullptr;
    void init();
    void switchConnectMode(bool isOn);
    void switchEditMode(bool isOn);
    void switchEditButtonState(bool isOn);
};
#endif // MAINWINDOW_H

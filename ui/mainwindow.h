#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidgetItem>
#include <QLabel>
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
    void onTableCellChanged(int nRow, int nCol);
    void onEditCheckBoxClicked(bool isOn);

    void onModbusStateChanged(bool isOn);

private:
//    strList << tr("Name") << tr("Value") << tr("Read") << tr("Write")
//            << tr("Block") << tr("Address") << tr("Ratio") << tr("BatchRead") << tr("BatchWrite");
    enum CellType{ctName=1000,ctValue,ctRead,ctWrite,ctBlock,ctAddress,ctRatio,ctBatchRead,ctBatchWrite};
    enum FieldColNum{colName=0,colValue,colRead,colWrite,colBlock,colAddress,colRatio,colBatchRead,colBatchWrite};
    Ui::MainWindow *ui;
    ModbusHandler* m_pModbusHandler = nullptr;
    QLabel* m_pCurRowLabel;
    void init();
    void initTableWidget();
    void createItemsARow(int nRow);
    void switchTableReadWriteState(int nRow);
    void switchConnectMode(bool isOn);
    void switchEditGroupBox(bool isOn);
    void switchEditButtonState(bool isOn);
    void switchReadWriteButtonState(int nRow);
};
#endif // MAINWINDOW_H

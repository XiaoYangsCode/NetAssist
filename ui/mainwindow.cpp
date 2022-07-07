#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QDebug>
#include <QPushButton>
#include <QComboBox>
#include "comboboxdelegate.h"
#include "lineeditdelegate.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    init();
}

MainWindow::~MainWindow()
{
    delete m_pCurRowLabel;
    delete m_pModbusHandler;
    delete ui;
}

void MainWindow::init()
{
    setWindowTitle(tr("NetAssist"));
    setFixedSize(990, 610);
    setWindowIcon(QIcon(":/icon/icon.png"));

    ui->ipLineEdit->setInputMask("000.000.000.000:0000;");
    ui->ipLineEdit->setText("127.0.0.1:5020");
    switchConnectMode(false);

    initTableWidget();

    // ui->editCheckBox->setChecked(true);
    // switchEditGroupBox(true);
    ui->readWriteGroupBox->setEnabled(false);
    switchEditButtonState(false);

    // TODO
    ui->batchReadPushButton->setEnabled(false);
    ui->batchWritePushButton->setEnabled(false);
    switchReadWriteButtonState(-1);

    // status bar
    m_pCurRowLabel = new QLabel(this);
    m_pCurRowLabel->setText(tr("Not Choose"));
    ui->statusbar->addWidget(m_pCurRowLabel);

    m_pModbusHandler = new ModbusHandler(this);
    connect(m_pModbusHandler, &ModbusHandler::modbusStateChanged, this, &MainWindow::onModbusStateChanged);
    connect(m_pModbusHandler, &ModbusHandler::modbusReceive, this, &MainWindow::onModbusReceive);

    connect(ui->connectPushButton, &QPushButton::clicked, this, &MainWindow::onConnectButtonClicked);
    connect(ui->readPushButton, &QPushButton::clicked, this, &MainWindow::onReadButtonClicked);
    connect(ui->appendPushButton, &QPushButton::clicked, this, &MainWindow::onAppendRowButtonClicked);
    connect(ui->removePushButton, &QPushButton::clicked, this, &MainWindow::onRemoveRowButtonClicked);
    connect(ui->insertPushButton, &QPushButton::clicked, this, &MainWindow::onInsertRowButtonClicked);
}

void MainWindow::initTableWidget()
{
    QStringList strList;
    strList << tr("Name") << tr("Value") << tr("Read") << tr("Write")
            << tr("Block") << tr("Address") << tr("Ratio") << tr("BatchRead") << tr("BatchWrite");
    ui->tableWidget->setColumnCount(strList.count());
    ui->tableWidget->setHorizontalHeaderLabels(strList);
    ui->tableWidget->clearContents();
    ui->tableWidget->setAlternatingRowColors(true);

    // block
    ComboBoxDelegate* pComboBoxDelegate = new ComboBoxDelegate(this);
    QStringList strListItems;
    strListItems << tr("DiscreteInput") << tr("Coil") << tr("InputRegister") << tr("HoldingRegister");
    pComboBoxDelegate->setItems(strListItems);
    ui->tableWidget->setItemDelegateForColumn(MainWindow::colBlock, pComboBoxDelegate);
    ui->tableWidget->setColumnWidth(MainWindow::colBlock, 150);

    // read and write
    ui->tableWidget->setColumnWidth(MainWindow::colRead, 50);
    ui->tableWidget->setColumnWidth(MainWindow::colWrite, 50);
    // value
    LineEditDelegate* pLineEditDelegate = new LineEditDelegate(this);
    ui->tableWidget->setItemDelegateForColumn(MainWindow::colValue, pLineEditDelegate);
    ui->tableWidget->setColumnWidth(MainWindow::colValue, 100);
    // name
    ui->tableWidget->setColumnWidth(MainWindow::colName, 120);

    // address
    LineEditDelegate* pLineEditDelegate1 = new LineEditDelegate(this);
    ui->tableWidget->setItemDelegateForColumn(MainWindow::colAddress, pLineEditDelegate1);
    ui->tableWidget->setColumnWidth(MainWindow::colAddress, 100);

    // batch read and write TODO
    ui->tableWidget->setColumnHidden(MainWindow::colBatchRead, true);
    ui->tableWidget->setColumnHidden(MainWindow::colBatchWrite, true);

    // ratio TODO
    ui->tableWidget->setColumnHidden(MainWindow::colRatio, true);

    connect(ui->tableWidget, &QTableWidget::currentCellChanged, this, &MainWindow::onTableCurrentCellChanged);
    connect(ui->tableWidget, &QTableWidget::cellChanged, this, &MainWindow::onTableCellChanged);
}

void MainWindow::createItemsARow(int nRow)
{
    QTableWidgetItem* pItem;
    // name
    pItem = new QTableWidgetItem(MainWindow::ctName);
    pItem->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui->tableWidget->setItem(nRow, MainWindow::colName, pItem);
    // value
    pItem = new QTableWidgetItem("--", MainWindow::ctValue);
    pItem->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui->tableWidget->setItem(nRow, MainWindow::colValue, pItem);
    // pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);
    // read
    pItem = new QTableWidgetItem("-", MainWindow::ctRead);
    pItem->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui->tableWidget->setItem(nRow, MainWindow::colRead, pItem);
    pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);
    // write
    pItem = new QTableWidgetItem("-", MainWindow::ctWrite);
    pItem->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui->tableWidget->setItem(nRow, MainWindow::colWrite, pItem);
    pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);
    // block
    pItem = new QTableWidgetItem(MainWindow::ctBlock);
    pItem->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui->tableWidget->setItem(nRow, MainWindow::colBlock, pItem);
//    QComboBox* pComboBox = new QComboBox();
//    pComboBox->addItems({tr("DiscreteInput"), tr("Coil"), tr("InputRegister"), tr("HoldingRegister")tr("HoldingRegister")});
//    ui->tableWidget->setCellWidget(nRow, MainWindow::colBlock, pComboBox);
//    ui->tableWidget->resizeColumnToContents(MainWindow::colBlock);
    // adress
    pItem = new QTableWidgetItem(MainWindow::ctAddress);
    pItem->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui->tableWidget->setItem(nRow, MainWindow::colAddress, pItem);
    // ratio
    pItem = new QTableWidgetItem("1", MainWindow::ctRatio);
    pItem->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui->tableWidget->setItem(nRow, MainWindow::colRatio, pItem);
    // batchread
    pItem = new QTableWidgetItem(tr("IsRead"), MainWindow::ctBatchRead);
    pItem->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    pItem->setCheckState(Qt::Unchecked);
    ui->tableWidget->setItem(nRow, MainWindow::colBatchRead, pItem);
    pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);
    // batchwrite
    pItem = new QTableWidgetItem(tr("IsWrite"), MainWindow::ctBatchWrite);
    pItem->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    pItem->setCheckState(Qt::Unchecked);
    ui->tableWidget->setItem(nRow, MainWindow::colBatchWrite, pItem);
    pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);
}

void MainWindow::switchTableReadWriteState(int nRow)
{
    if (nRow < 0 || nRow >= ui->tableWidget->rowCount())
    {
        return;
    }

    auto pItem = ui->tableWidget->item(nRow, MainWindow::colBlock);
    auto pItemRead = ui->tableWidget->item(nRow, MainWindow::colRead);
    auto pItemWrite = ui->tableWidget->item(nRow, MainWindow::colWrite);
    QString strBlockType = pItem->text();
    if (strBlockType.isEmpty())
    {
        pItemRead->setText("-");
        pItemWrite->setText("-");
        return;
    }
    pItemRead->setText(tr("Yes"));
    bool bWrite = m_pModbusHandler->getWriteStateByBlock(strBlockType);
    if (bWrite)
    {
        pItemWrite->setText(tr("Yes"));
    }
    else
    {
        pItemWrite->setText(tr("No"));
    }
}

void MainWindow::switchConnectMode(bool isOn)
{
    if (isOn)
    {
        ui->connectPushButton->setText(tr("Disconnect"));
        ui->ipLineEdit->setEnabled(false);
        ui->slaveSpinBox->setEnabled(false);
        ui->readWriteGroupBox->setEnabled(true);
    }
    else
    {
        ui->connectPushButton->setText(tr("Connect"));
        ui->ipLineEdit->setEnabled(true);
        ui->slaveSpinBox->setEnabled(true);
        ui->readWriteGroupBox->setEnabled(false);
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

void MainWindow::switchReadWriteButtonState(int nRow)
{
    if (nRow == -1)
    {
        ui->readPushButton->setEnabled(false);
        ui->writePushButton->setEnabled(false);
        return;
    }

    auto pItem = ui->tableWidget->item(nRow, MainWindow::colBlock);
    QString strBlockType = pItem->text();
    ui->readPushButton->setEnabled(!strBlockType.isEmpty());
    bool bWrite = m_pModbusHandler->getWriteStateByBlock(strBlockType);
    ui->writePushButton->setEnabled(bWrite);
}

void MainWindow::onConnectButtonClicked()
{
    m_pModbusHandler->tryConnect(ui->ipLineEdit->text());
}

void MainWindow::onReadButtonClicked()
{
    if (m_nCurRow == -1)
        return;

    auto pBlockItem = ui->tableWidget->item(m_nCurRow, MainWindow::colBlock);
    auto pAddressItem = ui->tableWidget->item(m_nCurRow, MainWindow::colAddress);
    m_pModbusHandler->tryRead(pBlockItem->text(), pAddressItem->text(), ui->slaveSpinBox->value());
}

void MainWindow::onAppendRowButtonClicked()
{
    int nRowCount = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(nRowCount);
    createItemsARow(nRowCount);
}

void MainWindow::onRemoveRowButtonClicked()
{
    ui->tableWidget->removeRow(ui->tableWidget->currentRow());
    ui->tableWidget->setCurrentCell(-1, -1);
}

void MainWindow::onInsertRowButtonClicked()
{
    int nCurRow = ui->tableWidget->currentRow();
    ui->tableWidget->insertRow(nCurRow);
    createItemsARow(nCurRow);
}

void MainWindow::onTableCurrentCellChanged(int nCurRow, int nCurCol, int nPreRow, int nPreCol)
{
    if (nCurRow >= 0)
        m_pCurRowLabel->setText(tr("Current Row: %1").arg(nCurRow + 1));
    else
        m_pCurRowLabel->setText(tr("Not Choose"));
    m_nCurRow = nCurRow;

    switchEditButtonState(nCurRow != -1);
    if (nCurRow != nPreRow)
    {
        switchReadWriteButtonState(nCurRow);
    }
}

void MainWindow::onTableCellChanged(int nRow, int nCol)
{
    if (nCol == MainWindow::colBlock)
    {
        switchTableReadWriteState(nRow);
        switchReadWriteButtonState(nRow);
    }
}

void MainWindow::onModbusStateChanged(bool isOn)
{
    switchConnectMode(isOn);
}

void MainWindow::onModbusReceive(QString sBlockType, int nAddress, QVector<quint16> buffer)
{
    qDebug() << sBlockType << nAddress << QString::number(buffer.value(0), 16);
    for (int i = 0; i < ui->tableWidget->rowCount(); ++i)
    {
        auto pBlockItem = ui->tableWidget->item(i, MainWindow::colBlock);
        auto pAddressItem = ui->tableWidget->item(i, MainWindow::colAddress);
        auto pValueItem = ui->tableWidget->item(i, MainWindow::colValue);
        int nItemAddress = m_pModbusHandler->getAddressByString(pAddressItem->text());
        if (sBlockType == pBlockItem->text() &&
            nAddress == nItemAddress)
        {
            QString sValue = m_pModbusHandler->getValueStringByNum(buffer.value(0));
            pValueItem->setText(sValue);
        }
    }
}





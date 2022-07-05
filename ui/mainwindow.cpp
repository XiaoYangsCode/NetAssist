#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QDebug>
#include <QPushButton>
#include <QComboBox>
#include "comboboxdelegate.h"


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
    ui->tableWidget->setColumnWidth(MainWindow::colValue, 80);
    // name
    ui->tableWidget->setColumnWidth(MainWindow::colName, 120);

    // batch read and write TODO
    ui->tableWidget->setColumnHidden(MainWindow::colBatchRead, true);
    ui->tableWidget->setColumnHidden(MainWindow::colBatchWrite, true);

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
    pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);
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

void MainWindow::switchReadWriteState(int nRow, QString strBlockType)
{
    if (nRow < 0 || nRow >= ui->tableWidget->rowCount())
    {
        return;
    }

    auto pItemRead = ui->tableWidget->item(nRow, MainWindow::colRead);
    auto pItemWrite = ui->tableWidget->item(nRow, MainWindow::colWrite);

    if (strBlockType == tr("DiscreteInput"))
    {
        pItemRead->setText(tr("Yes"));
        pItemWrite->setText(tr("No"));
    }
    else if (strBlockType == tr("Coil"))
    {
        pItemRead->setText(tr("Yes"));
        pItemWrite->setText(tr("Yes"));
    }
    else if (strBlockType == tr("InputRegister"))
    {
        pItemRead->setText(tr("Yes"));
        pItemWrite->setText(tr("No"));
    }
    else if (strBlockType == tr("HoldingRegister"))
    {
        pItemRead->setText(tr("Yes"));
        pItemWrite->setText(tr("Yes"));
    }
    else
    {
        pItemRead->setText("-");
        pItemWrite->setText("-");
    }
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
    qDebug() << "current row:" << nCurRow;
    switchEditButtonState(nCurRow != -1);
}

void MainWindow::onTableCellChanged(int nRow, int nCol)
{
    if (nCol == MainWindow::colBlock)
    {
//        qDebug() << nRow << nCol;
        qDebug() << "ColBlock";
        auto pItem = ui->tableWidget->item(nRow, nCol);
//        qDebug() << pItem->text();
//        qDebug() << pItem->data(Qt::EditRole);
        switchReadWriteState(nRow, pItem->text());
    }
}

void MainWindow::onEditCheckBoxClicked(bool isOn)
{
    switchEditMode(isOn);
}

void MainWindow::onModbusStateChanged(bool isOn)
{
    switchConnectMode(isOn);
}





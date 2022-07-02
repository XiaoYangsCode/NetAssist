#include "modbushandler.h"
#include <QDebug>
#include <QUrl>
#include <QVariant>

ModbusHandler::ModbusHandler(QObject *parent)
    : QObject{parent}
{
    init();
}

ModbusHandler::~ModbusHandler()
{
    delete m_pModbusClient;
}

void ModbusHandler::onStateChanged()
{
    if (!m_pModbusClient)
        return;
    if (m_pModbusClient->state() == QModbusDevice::ConnectedState)
        emit modbusStateChanged(true);
    else
        emit modbusStateChanged(false);
}


void ModbusHandler::init()
{
    m_pModbusClient = new QModbusTcpClient();
    connect(m_pModbusClient, &QModbusClient::stateChanged, this, &ModbusHandler::onStateChanged);
}

bool ModbusHandler::tryConnect(QString sIpAddress)
{
    if(!m_pModbusClient)
    {
        return false;
    }

    if (m_pModbusClient->state() != QModbusDevice::ConnectedState)
    {
        const QUrl url = QUrl::fromUserInput(sIpAddress);
        m_pModbusClient->setConnectionParameter(QModbusDevice::NetworkAddressParameter, url.host());
        m_pModbusClient->setConnectionParameter(QModbusDevice::NetworkPortParameter, url.port());
        m_pModbusClient->setTimeout(1000);
        m_pModbusClient->setNumberOfRetries(3);

        if (!m_pModbusClient->connectDevice())
        {
            qDebug()<< "连接modbus设备失败";
            return false;
        }
        else
        {
            qDebug()<< "成功连接到modbs设备";
            return true;
        }
    }
    else
    {
        m_pModbusClient->disconnectDevice();
        return false;
    }
}



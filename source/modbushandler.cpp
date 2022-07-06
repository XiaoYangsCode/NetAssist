#include "modbushandler.h"
#include <QDebug>
#include <QUrl>
#include <QVariant>

ModbusHandler::ModbusHandler(QObject *parent)
    : QObject{parent}
{
    reset();
}

ModbusHandler::~ModbusHandler()
{
    if (m_pModbusClient)
        m_pModbusClient->disconnectDevice();
    delete m_pModbusClient;
}

void ModbusHandler::onStateChanged()
{
    if (!m_pModbusClient)
        return;
    if (m_pModbusClient->state() == QModbusDevice::ConnectedState)
    {
        qDebug() << tr("State Connected");
        emit modbusStateChanged(true);
    }
    else
    {
        qDebug() << tr("State Disconnected");
        emit modbusStateChanged(false);
    }
}

void ModbusHandler::onErrorOccurred()
{
    if (!m_pModbusClient)
        return;
    qDebug() << tr("Connect failed: ") << m_pModbusClient->errorString();
}

void ModbusHandler::onReadReady()
{
    QVector<quint16> buffer;
    QModbusReply* pReply = qobject_cast<QModbusReply*>(sender());
    if (!pReply)
    {
        qDebug() << tr("Reply is null");
        return;
    }
//    const QModbusResponse response = pReply->rawResult();
//    qDebug() << "respones: " << response.data().toHex();
    if (pReply->error() == QModbusDevice::NoError)
    {
        qDebug() << tr("Receive data");
        const QModbusDataUnit unit = pReply->result();
        for (quint16 i=0; i < unit.valueCount(); ++i)
        {
            // TODO
//            buffer.append(static_cast<quint16>(unit.value(i)));
        }
    }
    qDebug() << tr("Result: ") << buffer;
    pReply->deleteLater();
}


void ModbusHandler::reset()
{
    if (m_pModbusClient)
    {
        m_pModbusClient->disconnectDevice();
        delete m_pModbusClient;
        m_pModbusClient = nullptr;
    }
    m_pModbusClient = new QModbusTcpClient(this);
    connect(m_pModbusClient, &QModbusClient::stateChanged, this, &ModbusHandler::onStateChanged);
    connect(m_pModbusClient, &QModbusClient::errorOccurred, this, &ModbusHandler::onErrorOccurred);
}

bool ModbusHandler::tryConnect(QString sIpAddress) const
{
    if (!m_pModbusClient)
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
            qDebug() << tr("Connection initialize failed");
            return false;
        }
        else
        {
            qDebug() << tr("Connection initialize success");
            return true;
        }
    }
    else
    {
        m_pModbusClient->disconnectDevice();
        return false;
    }
}

bool ModbusHandler::tryRead() const
{
    if (m_pModbusClient->state() != QModbusDevice::ConnectedState)
    {
        return false;
    }

    QModbusDataUnit readUnit(QModbusDataUnit::Coils, 0, 2);

//    QModbusRequest message( QModbusRequest::ReadCoils, quint8(0x00), quint8(0x02));
//    qDebug() << "request: " << message.data().toHex();
    if (QModbusReply *pReply = m_pModbusClient->sendReadRequest(readUnit, 0))
    {
        if (!pReply->isFinished())
        {
            qDebug() << tr("Send read message");
            connect(pReply, &QModbusReply::finished, this, &ModbusHandler::onReadReady);
            return true;
        }
        else
        {
            qDebug() << tr("Relay is finished");
            delete pReply;
            return false;
        }
    }
    else
    {
        qDebug() << tr("Fail to send read request");
        return false;
    }

}

bool ModbusHandler::getWriteStateByBlock(const QString &strBlockType) const
{
    if (strBlockType == tr("DiscreteInput"))
    {
        return false;
    }
    else if (strBlockType == tr("Coil"))
    {
        return true;
    }
    else if (strBlockType == tr("InputRegister"))
    {
        return false;
    }
    else if (strBlockType == tr("HoldingRegister"))
    {
        return true;
    }
    else
    {
        return false;
    }
}









#include "modbushandler.h"
#include <QDebug>
#include <QUrl>
#include <QVariant>

QRegularExpression ModbusHandler::s_removeSpaceReg = QRegularExpression("\\s");

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
        // if (unit.registerType() == QModbusDataUnit::Coils ||
            // unit.registerType() == QModbusDataUnit::InputRegisters)
        // {
            // qDebug() << pReply->rawResult();
            // qDebug() << pReply->rawResult().data();
        // }

        for (quint16 i=0; i < unit.valueCount(); ++i)
        {
           buffer.append(static_cast<quint16>(unit.value(i)));
        }

        qDebug() << tr("Result: ") << buffer;
        qDebug() << unit.startAddress();

        if (buffer.count() > 0)
            emit modbusReceive(getBlockStringByType(unit.registerType()), unit.startAddress(), buffer);
    }
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

bool ModbusHandler::tryRead(QString sBlockType, QString sAddress, int nSlaveId) const
{
    if (m_pModbusClient->state() != QModbusDevice::ConnectedState)
    {
        return false;
    }

    QModbusDataUnit readUnit = readRequest(sBlockType, sAddress);

    // QModbusDataUnit readUnit(QModbusDataUnit::Coils, 0, 2);
//    QModbusRequest message( QModbusRequest::ReadCoils, quint8(0x00), quint8(0x02));
//    qDebug() << "request: " << message.data().toHex();
    if (QModbusReply *pReply = m_pModbusClient->sendReadRequest(readUnit, nSlaveId))
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

QModbusDataUnit ModbusHandler::readRequest(QString sBlockType, QString sAddress) const
{
    QModbusDataUnit::RegisterType eBlockType = getBlockTypeByString(sBlockType);
    if (eBlockType == QModbusDataUnit::RegisterType::Invalid)
    {
        qDebug() << "Block Type is null";
        return QModbusDataUnit();
    }

    int nDecAddress = getAddressByString(sAddress);
    return QModbusDataUnit(eBlockType, nDecAddress, 1);
}

bool ModbusHandler::getWriteStateByBlock(const QString &sBlockType) const
{
    if (sBlockType == tr("DiscreteInput"))
    {
        return false;
    }
    else if (sBlockType == tr("Coil"))
    {
        return true;
    }
    else if (sBlockType == tr("InputRegister"))
    {
        return false;
    }
    else if (sBlockType == tr("HoldingRegister"))
    {
        return true;
    }
    else
    {
        return false;
    }
}

QModbusDataUnit::RegisterType ModbusHandler::getBlockTypeByString(QString& sBlockType) const
{
    if (sBlockType == tr("DiscreteInput"))
    {
        return QModbusDataUnit::RegisterType::DiscreteInputs;
    }
    else if (sBlockType == tr("Coil"))
    {
        return QModbusDataUnit::RegisterType::Coils;
    }
    else if (sBlockType == tr("InputRegister"))
    {
        return QModbusDataUnit::RegisterType::InputRegisters;
    }
    else if (sBlockType == tr("HoldingRegister"))
    {
        return QModbusDataUnit::RegisterType::HoldingRegisters;
    }
    else
    {
        return QModbusDataUnit::RegisterType::Invalid;
    }
}

QString ModbusHandler::getBlockStringByType(QModbusDataUnit::RegisterType eBlockType) const
{
    if (eBlockType == QModbusDataUnit::RegisterType::DiscreteInputs)
    {
        return tr("DiscreteInput");
    }
    else if (eBlockType == QModbusDataUnit::RegisterType::Coils)
    {
        return tr("Coil");
    }
    else if (eBlockType == QModbusDataUnit::RegisterType::InputRegisters)
    {
        return tr("InputRegister");
    }
    else if (eBlockType == QModbusDataUnit::RegisterType::HoldingRegisters)
    {
        return tr("HoldingRegister");
    }
    else
    {
        return "";
    }
}

int ModbusHandler::getAddressByString(QString sAddress) const
{
    QString sHexAddress = sAddress.remove(ModbusHandler::s_removeSpaceReg);
    bool ok;
    int nDecAddress = sHexAddress.toInt(&ok, 16);
    if (!ok)
        qDebug() << "sAddress is error";
    return nDecAddress;
}

QString ModbusHandler::getValueStringByNum(quint16 nValue)
{
    QString sValue = QString("%1").arg(nValue, 4, 16, QLatin1Char('0'));
    return sValue.insert(2, ' ');
}









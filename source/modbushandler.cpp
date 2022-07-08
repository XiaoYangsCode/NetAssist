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
        emit modbusLog(tr("Modbus Connect State: Connected"));
        emit modbusStateChanged(true);
    }
    else if (m_pModbusClient->state() == QModbusDevice::UnconnectedState)
    {
        emit modbusLog(tr("Modbus Connect State: Unconnected"));
        emit modbusStateChanged(false);
    }
    else if (m_pModbusClient->state() == QModbusDevice::ConnectingState)
    {
        emit modbusLog(tr("Modbus Connect State: Connecting"));
        emit modbusStateChanged(false);
    }
    else if (m_pModbusClient->state() == QModbusDevice::ClosingState)
    {
        emit modbusLog(tr("Modbus Connect State: Closing"));
        emit modbusStateChanged(false);
    }
    else
    {
        emit modbusLog(tr("Modbus Connect State: Unknown State"));
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
        qDebug() << tr("Read reply is null");
        return;
    }
//    const QModbusResponse response = pReply->rawResult();
//    qDebug() << "respones: " << response.data().toHex();
    if (pReply->error() == QModbusDevice::NoError)
    {
        const QModbusDataUnit unit = pReply->result();

        for (quint16 i=0; i < unit.valueCount(); ++i)
        {
           buffer.append(static_cast<quint16>(unit.value(i)));
        }

        QString sBlockType = getBlockStringByType(unit.registerType());
        QString sAddress = getStringByValue(unit.startAddress());
        QString sBuffer;
        sBuffer.append("(");
        foreach(quint16 nValue, buffer)
        {
            sBuffer.append(QString::number(nValue));
            sBuffer.append(", ");
        }
        sBuffer.append(")");

        emit modbusLog(tr("Receive read data: BlockType %1, Start Address %2, Buffer %3")
                       .arg(sBlockType, sAddress, sBuffer));

        if (buffer.count() > 0)
            emit modbusReceive(sBlockType, sAddress, buffer);
    }
    else
    {
        emit modbusLog(tr("Receive read data: fail"));
    }
    pReply->deleteLater();
}

void ModbusHandler::onWriteReady()
{
    QModbusReply* pReply = qobject_cast<QModbusReply*>(sender());
    if (!pReply)
    {
        qDebug() << tr("Write reply is null");
        return;
    }
    if (pReply->error() == QModbusDevice::ProtocolError)
    {
        QString sMsg = tr("Write response error: %1 (Modbus exception: 0x%2)").
                arg(pReply->errorString()).arg(pReply->rawResult().exceptionCode(), -1, 16);
        qDebug() << sMsg;
        emit modbusLog(tr("Receive write data: fail"));
    }
    else if (pReply->error() != QModbusDevice::NoError)
    {
        QString sMsg = tr("Write response error: %1 (code: 0x%2)").
                arg(pReply->errorString()).arg(pReply->error(), -1, 16);
        qDebug() << sMsg;
        emit modbusLog(tr("Receive write data: fail"));
    }
    {
        const QModbusDataUnit unit = pReply->result();
        QString sBlockType = getBlockStringByType(unit.registerType());
        QString sAddress = getStringByValue(unit.startAddress());
        emit modbusLog(tr("Receive write data: BlockType %1, Start Address %2")
                       .arg(sBlockType, sAddress));
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
            return true;
        }
    }
    else
    {
        m_pModbusClient->disconnectDevice();
        return false;
    }
}

bool ModbusHandler::tryRead(QString sBlockType, QString sAddress, int nSlaveId)
{
    if (m_pModbusClient->state() != QModbusDevice::ConnectedState)
    {
        return false;
    }

    QModbusDataUnit readUnit = readRequest(sBlockType, sAddress);

    // QModbusDataUnit readUnit(QModbusDataUnit::Coils, 0, 2);
//    QModbusRequest message( QModbusRequest::ReadCoils, quint8(0x00), quint8(0x02));
    emit modbusLog(tr("Send read message: BlockType %1, Start Address %2, SlaveId %3")
                   .arg(sBlockType, sAddress).arg(nSlaveId));
    if (QModbusReply *pReply = m_pModbusClient->sendReadRequest(readUnit, nSlaveId))
    {
        if (!pReply->isFinished())
        {
            connect(pReply, &QModbusReply::finished, this, &ModbusHandler::onReadReady);
            return true;
        }
        else
        {
            qDebug() << tr("Read relay is finished");
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

bool ModbusHandler::tryWrite(QString sBlockType, QString sAddress, QString sValue, int nSlaveId)
{
    if (m_pModbusClient->state() != QModbusDevice::ConnectedState)
    {
        return false;
    }

    QModbusDataUnit writeUnit = writeRequest(sBlockType, sAddress, sValue);

    emit modbusLog(tr("Send write message: BlockType %1, Start Address %2, Value %3, SlaveId %4")
                   .arg(sBlockType, sAddress, sValue).arg(nSlaveId));

    if (QModbusReply *pReply = m_pModbusClient->sendWriteRequest(writeUnit, nSlaveId))
    {
        if (!pReply->isFinished())
        {
            connect(pReply, &QModbusReply::finished, this, &ModbusHandler::onWriteReady);
            return true;
        }
        else
        {
            qDebug() << tr("Write relay is finished");
            delete pReply;
            return false;
        }
    }
    else
    {
        qDebug() << tr("Fail to send write request");
        return false;
    }


}

QModbusDataUnit ModbusHandler::readRequest(QString sBlockType, QString sAddress) const
{
    QModbusDataUnit::RegisterType eBlockType = getBlockTypeByString(sBlockType);
    if (eBlockType == QModbusDataUnit::RegisterType::Invalid)
    {
        qDebug() << "Read Block Type is error";
        return QModbusDataUnit();
    }

    int nDecAddress = getValueByString(sAddress);
    return QModbusDataUnit(eBlockType, nDecAddress, 1);
}

QModbusDataUnit ModbusHandler::writeRequest(QString sBlockType, QString sAddress, QString sValue) const
{
    QModbusDataUnit::RegisterType eBlockType = getBlockTypeByString(sBlockType);
    if (eBlockType == QModbusDataUnit::RegisterType::Invalid)
    {
        qDebug() << "Write Block Type is error";
        return QModbusDataUnit();
    }

    int nDecAddress = getValueByString(sAddress);
    int nValue = getValueByString(sValue);
    auto unit = QModbusDataUnit(eBlockType, nDecAddress, 1);
    unit.setValue(0, nValue);
    return unit;
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

int ModbusHandler::getValueByString(QString sValue) const
{
    QString sHexValue = sValue.remove(ModbusHandler::s_removeSpaceReg);
    bool ok;
    int nDecValue = sHexValue.toInt(&ok, 16);
    if (!ok)
        qDebug() << "sValue is error";
    return nDecValue;
}

QString ModbusHandler::getStringByValue(quint16 nValue) const
{
    QString sValue = QString("%1").arg(nValue, 4, 16, QLatin1Char('0'));
    return sValue.insert(2, ' ');
}









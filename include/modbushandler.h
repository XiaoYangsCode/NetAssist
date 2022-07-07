#ifndef MODBUSHANDLER_H
#define MODBUSHANDLER_H

#include <QObject>
#include <QModbusTcpClient>
#include <QModbusDataUnit>
#include <QRegularExpression>

class ModbusHandler : public QObject
{
    Q_OBJECT
public:
    explicit ModbusHandler(QObject *parent = nullptr);
    ~ModbusHandler();
    bool tryConnect(QString sIpAddress) const;
    bool tryRead(QString sBlockType, QString sAddress, int nSlaveId) const;
    QModbusDataUnit readRequest(QString sBlockType, QString sAddress) const;

    // enum BlockType {DiscreteInput, Coil, InputRegister, HoldingRegister};
    bool getWriteStateByBlock(const QString& sBlockType) const;
    QModbusDataUnit::RegisterType getBlockTypeByString(QString& sBlockType) const;
    QString getBlockStringByType(QModbusDataUnit::RegisterType eBlockType) const;
    int getAddressByString(QString sAddress) const;
    QString getValueStringByNum(quint16 nValue);

private:
    QModbusClient* m_pModbusClient = nullptr;
    void reset();

signals:
    void modbusStateChanged(bool isConnected);
    void modbusReceive(QString eBlockType, int nAddress, QVector<quint16> buffer);

private slots:
    void onStateChanged();
    void onErrorOccurred();
    void onReadReady();

public:
    static QRegularExpression s_removeSpaceReg;

};

#endif // MODBUSHANDLER_H

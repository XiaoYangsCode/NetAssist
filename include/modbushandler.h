#ifndef MODBUSHANDLER_H
#define MODBUSHANDLER_H

#include <QObject>
#include <QModbusTcpClient>

class ModbusHandler : public QObject
{
    Q_OBJECT
public:
    explicit ModbusHandler(QObject *parent = nullptr);
    ~ModbusHandler();
    bool tryConnect(QString sIpAddress) const;
    bool tryRead() const;

    enum BlockType {DiscreteInput, Coil, InputRegister, HoldingRegister};
    bool getWriteStateByBlock(const QString& strBlockType) const;

private:
    QModbusClient* m_pModbusClient = nullptr;
    void reset();

signals:
    void modbusStateChanged(bool isConnected);

private slots:
    void onStateChanged();
    void onErrorOccurred();
    void onReadReady();

};

#endif // MODBUSHANDLER_H

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
    bool tryConnect(QString sIpAddress);

private:
    QModbusClient* m_pModbusClient;
    void init();

signals:
    void modbusStateChanged(bool isConnected);

private slots:
    void onStateChanged();

};

#endif // MODBUSHANDLER_H

#ifndef MODBUSSERVER_HPP
#define MODBUSSERVER_HPP

#include <QObject>
#include <QHostAddress>
#include "RegisterBank.hpp"

namespace emulator::modbus {

class ModbusTcpServer;

class ModbusServer : public QObject {
    Q_OBJECT
public:
    explicit ModbusServer(emulator::registerbank::RegisterBank* regBank,
                          QObject* parent = nullptr);
    ~ModbusServer() override;

    bool start(const QString& address, uint16_t port);
    void stop();
    bool isRunning() const;

private:
    ModbusTcpServer* m_server;
};

}

#endif // MODBUSSERVER_HPP
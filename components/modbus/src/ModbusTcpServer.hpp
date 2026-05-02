#ifndef MODBUSTCPSERVER_HPP
#define MODBUSTCPSERVER_HPP

#include <QModbusTcpServer>
#include <QModbusDataUnit>
#include <QModbusResponse>
#include <QModbusPdu>
#include "RegisterBank.hpp"

namespace emulator::modbus {

class ModbusTcpServer : public QModbusTcpServer {
    Q_OBJECT
public:
    explicit ModbusTcpServer(emulator::registerbank::RegisterBank* regBank,
                                        QObject* parent = nullptr);

protected:
    bool writeData(const QModbusDataUnit &data) override;
    bool readData(QModbusDataUnit *data) const override;

private:
    emulator::registerbank::RegisterBank* registerBank_;
    static constexpr size_t MAX_REGISTERS = 1000;
};

}



#endif // MODBUSTCPSERVER_HPP

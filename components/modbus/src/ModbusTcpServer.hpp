#ifndef MODBUSTCPSERVER_HPP
#define MODBUSTCPSERVER_HPP

#include <QModbusTcpServer>
#include <QModbusDataUnit>
#include <QModbusResponse>
#include <QModbusPdu>

namespace emulator::modbus {

class RegisterBank {
public:
    ~RegisterBank() = default;

    // Чтение диапазонов
    QVector<bool> readCoils(int startAddr, int count) const {
        qInfo("Reading %d coils starting at %d", startAddr, count);
        QVector<bool> responce = {true, false, true, false};
        return responce;
    }

    QVector<bool> readDiscreteInputs(int startAddr, int count) const {
        qInfo("Reading %d DiscreteInputs starting at %d", startAddr, count);
        QVector<bool> responce = {true, false, true, false};
        return responce;
    }

    QVector<quint16> readInputRegisters(int startAddr, int count) const {
        qInfo("Reading %d InputRegisters starting at %d", startAddr, count);
        QVector<uint16_t> responce = {0x0123, 0x4567, 0x8910};
        return responce;
    }

    QVector<quint16> readHoldingRegisters(int startAddr, int count) const {
        qInfo("Reading %d HoldingRegisters starting at %d", startAddr, count);
        QVector<uint16_t> responce = {0x0123, 0x4567, 0x8910};
        return responce;
    }

    // Запись
    bool writeCoil(int addr, bool value) {
        qInfo("Writing coil with value %d at address %d", value, addr);
        return true;
    }

    bool writeCoils(int startAddr, const QVector<bool>& values) {
        qInfo("Writing coils");
        return true;
    }

    bool writeHoldingRegister(int addr, uint16_t value) {
        qInfo("Writing HoldingRegister with value %d at address %d", value, addr);
        return true;
    }

    bool writeHoldingRegisters(int startAddr, const QVector<uint16_t>& values) {
        qInfo("Writing HoldingRegisters");
        return true;
    }
};

class ModbusTcpServer : public QModbusTcpServer {
    Q_OBJECT
public:
    explicit ModbusTcpServer(RegisterBank* regBank,
                                        QObject* parent = nullptr);

protected:
    bool writeData(const QModbusDataUnit &data) override;
    bool readData(QModbusDataUnit *data) const override;

private:
    RegisterBank* registerBank_;
    static constexpr size_t MAX_REGISTERS = 1000;
};

}



#endif // MODBUSTCPSERVER_HPP

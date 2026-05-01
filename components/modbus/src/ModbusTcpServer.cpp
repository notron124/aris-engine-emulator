#include "ModbusTcpServer.hpp"
#include <QModbusExceptionResponse>
#include <QDebug>

namespace emulator::modbus {

ModbusTcpServer::ModbusTcpServer(RegisterBank* regBank,
                                 QObject* parent)
    : QModbusTcpServer(parent)
    , registerBank_(regBank)
{
    QModbusDataUnitMap map;
    map.insert(QModbusDataUnit::Coils, { QModbusDataUnit::Coils, 0, MAX_REGISTERS});
    map.insert(QModbusDataUnit::DiscreteInputs, { QModbusDataUnit::DiscreteInputs, 0, MAX_REGISTERS});
    map.insert(QModbusDataUnit::InputRegisters, { QModbusDataUnit::InputRegisters, 0, MAX_REGISTERS });
    map.insert(QModbusDataUnit::HoldingRegisters, { QModbusDataUnit::HoldingRegisters, 0, MAX_REGISTERS });
    setMap(map);
    setServerAddress(1);
}

bool ModbusTcpServer::readData(QModbusDataUnit *data) const {
    // const QModbusDataUnit::RegisterType regType = data->registerType();
    // const qsizetype valueCount = data->valueCount();

    // switch (regType) {
    // case QModbusDataUnit::Coils: {
    //     auto bits = registerBank_->readCoils(data->startAddress(), valueCount);
    //     for (qsizetype i = 0; i < bits.size(); ++i) {
    //         data->setValue(i, bits[i]);
    //     }
    //     return true;
    // }

    // case QModbusDataUnit::DiscreteInputs: {
    //     auto bits = registerBank_->readDiscreteInputs(data->startAddress(), valueCount);
    //     for (qsizetype i = 0; i < bits.size(); ++i) {
    //         data->setValue(i, bits[i]);
    //     }
    //     return true;
    // }

    // case QModbusDataUnit::InputRegisters: {
    //     auto bits = registerBank_->readInputRegisters(data->startAddress(), valueCount);
    //     for (qsizetype i = 0; i < bits.size(); ++i) {
    //         data->setValue(i, bits[i]);
    //     }
    //     return true;
    // }

    // case QModbusDataUnit::HoldingRegisters: {
    //     auto bits = registerBank_->readHoldingRegisters(data->startAddress(), valueCount);
    //     for (qsizetype i = 0; i < bits.size(); ++i) {
    //         data->setValue(i, bits[i]);
    //     }
    //     return true;
    // }

    // case QModbusDataUnit::Invalid: {
    //     qWarning("Invalid register type read requested");
    //     return false;
    // }

    // default:
    //     return false;
    // }

    return QModbusTcpServer::readData(data);
}

bool ModbusTcpServer::writeData(const QModbusDataUnit &data) {
    return QModbusTcpServer::writeData(data);
}

}



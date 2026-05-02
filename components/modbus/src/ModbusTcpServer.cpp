#include "ModbusTcpServer.hpp"
#include <QModbusExceptionResponse>
#include <QDebug>

namespace emulator::modbus {

ModbusTcpServer::ModbusTcpServer(emulator::registerbank::RegisterBank* regBank,
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

bool ModbusTcpServer::readData(QModbusDataUnit* data) const {
    const QModbusDataUnit::RegisterType regType = data->registerType();
    const qsizetype valueCount = data->valueCount();
    const qsizetype startAddr = data->startAddress();

    switch (regType) {
    case QModbusDataUnit::Coils: {
        auto bits = registerBank_->readCoils(startAddr, valueCount);
        for (qsizetype i = 0; i < bits.size(); ++i) {
            data->setValue(i, bits[i] ? 1 : 0);
        }
        return true;
    }

    case QModbusDataUnit::DiscreteInputs: {
        auto bits = registerBank_->readDiscreteInputs(startAddr, valueCount);
        for (qsizetype i = 0; i < bits.size(); ++i) {
            data->setValue(i, bits[i] ? 1 : 0);
        }
        return true;
    }

    case QModbusDataUnit::InputRegisters: {
        auto bits = registerBank_->readInputRegisters(startAddr, valueCount);
        for (qsizetype i = 0; i < bits.size(); ++i) {
            data->setValue(i, bits[i]);
        }
        return true;
    }

    case QModbusDataUnit::HoldingRegisters: {
        auto bits = registerBank_->readHoldingRegisters(startAddr, valueCount);
        for (qsizetype i = 0; i < bits.size(); ++i) {
            data->setValue(i, bits[i]);
        }
        return true;
    }

    case QModbusDataUnit::Invalid: {
        qWarning("Invalid register type read requested");
        return false;
    }

    default:
        return false;
    }

    return QModbusTcpServer::readData(data);
}

bool ModbusTcpServer::writeData(const QModbusDataUnit& data) {
    const QModbusDataUnit::RegisterType regType = data.registerType();
    const qsizetype startAddr = data.startAddress();
    const QList<uint16_t> payload = data.values();

    switch (regType) {
    case QModbusDataUnit::Coils: {
        QList<bool> coils;
        coils.reserve(data.valueCount());
        std::transform(payload.begin(), payload.end(), std::back_inserter(coils),
                       [](uint16_t value) {
                           return static_cast<bool>(value);
                       });
        registerBank_->writeCoils(startAddr, coils);
        return true;
    }

    case QModbusDataUnit::HoldingRegisters: {
        registerBank_->writeHoldingRegisters(startAddr, payload);
        return true;
    }

    case QModbusDataUnit::Invalid: {
        qWarning("Invalid register type read requested");
        return false;
    }

    default:
        return false;
    }

    return QModbusTcpServer::writeData(data);
}

}



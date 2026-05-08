#ifndef REGISTERBANK_HPP
#define REGISTERBANK_HPP

#include <QObject>
#include <QDebug>

namespace emulator::registerbank {

class RegisterBank : public QObject{
    Q_OBJECT
public:
    static constexpr size_t MAX_REGISTERS = 1000;

    RegisterBank() {
        coils_.fill(false, MAX_REGISTERS);
        discreteInputs_.fill(false, MAX_REGISTERS);
        inputRegisters_.fill(0, MAX_REGISTERS);
        holdingRegisters_.fill(0, MAX_REGISTERS);
    }

    ~RegisterBank() = default;

    // Чтение диапазонов
    QList<bool> readCoils(size_t startAddr, size_t count) const {
        return coils_.mid(startAddr, count);
    }

    QList<bool> readDiscreteInputs(size_t startAddr, size_t count) const {
        return discreteInputs_.mid(startAddr, count);
    }

    QList<uint16_t> readInputRegisters(size_t startAddr, size_t count) const {
        return inputRegisters_.mid(startAddr, count);
    }

    QList<uint16_t> readHoldingRegisters(size_t startAddr, size_t count) const {
        return holdingRegisters_.mid(startAddr, count);
    }

    // Запись
    bool writeCoil(size_t addr, bool value) {
        coils_[addr] = value;
        return true;
    }

    bool writeCoils(size_t startAddr, const QList<bool>& values) {
        for (size_t i = 0; i < values.size(); ++i) {
            coils_[startAddr + i] = values[i] ? 1 : 0;
        }

        return true;
    }

    bool writeHoldingRegister(size_t addr, uint16_t value) {
        holdingRegisters_[addr] = value;
        return true;
    }

    bool writeHoldingRegisters(size_t startAddr, const QList<uint16_t>& values) {
        std::copy(values.begin(), values.end(), holdingRegisters_.begin() + startAddr);
        return true;
    }

protected:
    QList<bool> coils_;
    QList<bool> discreteInputs_;
    QList<uint16_t> inputRegisters_;
    QList<uint16_t> holdingRegisters_;
};

}

#endif // REGISTERBANK_HPP

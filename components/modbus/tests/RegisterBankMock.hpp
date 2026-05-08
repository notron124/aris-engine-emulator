#ifndef REGISTERBANKMOCK_HPP
#define REGISTERBANKMOCK_HPP

#include "RegisterBank.hpp"

namespace emulator::modbus::tests {

class RegisterBankMock : public emulator::registerbank::RegisterBank {
public:
    void writeDiscreteInput(size_t addr, bool value) {
        discreteInputs_[addr] = value;
    }

    void writeInputRegister(size_t addr, uint16_t value) {
        inputRegisters_[addr] = value;
    }
};

}

#endif // REGISTERBANKMOCK_HPP

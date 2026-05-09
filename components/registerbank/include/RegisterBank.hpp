#ifndef REGISTERBANK_HPP
#define REGISTERBANK_HPP

#include <QDebug>
#include <QList>
#include <cstring>
#include <cstdint>
#include <optional>
#include <chrono>

#include "simulation/contracts/ExchangeContract.hpp"

namespace emulator::registerbank {

    using emulator::exchange::ClientInputSnapshot;
    using model::ModelInputs;
    using model::Limits;
    using exchange::ModelOutputSnapshot;
    using model::ModelOutputs;
    using emulator::simulation::SimulationCommand;
    using emulator::simulation::SimulationRequest;
    using emulator::simulation::SimulationMode;
    using emulator::simulation::SimulationState;
    using simulation::diagnostics::SimulationFaultCode;

    class RegisterBank{
    public:
        
        RegisterBank() {
            coils_.resize(numberOfCoils);
            coils_.fill(false);
            discreteInputs_.resize(numberOfDiscreteInputs);
            discreteInputs_.fill(false);
            inputRegisters_.resize(numberOfInputRegisters);
            inputRegisters_.fill(0);
            holdingRegisters_.resize(numberOfHoldingRegisters);
            holdingRegisters_.fill(0);
        }

        ~RegisterBank() = default;

        ClientInputSnapshot RegBankSendInfo();
        bool RegBankTakeInfo(const ModelOutputSnapshot& InputData);

        QList<bool> readCoils(uint16_t startAddr, uint16_t count) const;
        QList<bool> readDiscreteInputs(uint16_t startAddr, uint16_t count) const;
        QList<uint16_t> readInputRegisters(uint16_t startAddr, uint16_t count) const;
        QList<uint16_t> readHoldingRegisters(uint16_t startAddr, uint16_t count) const;

        bool writeCoil(uint16_t addr, bool value);
        bool writeCoils(uint16_t startAddr, const QList<bool>& values);
        bool writeHoldingRegister(uint16_t addr, uint16_t value);
        bool writeHoldingRegisters(uint16_t startAddr, const QList<uint16_t>& values);

        // InputRegisters offset
        static constexpr uint16_t T_cool = 0;
        static constexpr uint16_t P_oil = 4;
        static constexpr uint16_t omega_ICE_prir = 8;
        static constexpr uint16_t omega_ICE_run = 12;
        static constexpr uint16_t T_AD = 16;
        static constexpr uint16_t T_ballast = 20;
        static constexpr uint16_t M_AD = 24;
        static constexpr uint16_t f_AD = 28;
        static constexpr uint16_t revision = 32;
        static constexpr uint16_t sourceInputRevision = 36;
        static constexpr uint16_t modelTime = 40;
        static constexpr uint16_t timestamp_ir = 44;
        static constexpr uint16_t state_ir = 48;
        static constexpr uint16_t faultCode = 49;

        // HoldingRegisters offset
        static constexpr uint16_t T_cool_max = 0;
        static constexpr uint16_t P_oil_min = 4;
        static constexpr uint16_t P_oil_max = 8;
        static constexpr uint16_t omega_ICE_max_prir = 12;
        static constexpr uint16_t omega_ICE_max_run = 16;
        static constexpr uint16_t T_AD_max = 20;
        static constexpr uint16_t T_ballast_max = 24;
        static constexpr uint16_t f_AD_Input = 28;    
        static constexpr uint16_t M_AD_target = 32;
        static constexpr uint16_t revision_h = 36;
        static constexpr uint16_t simulationCommand = 40;
        static constexpr uint16_t simulationRequest = 41;
        static constexpr uint16_t simulationMode = 42;

        // Coils offset
        static constexpr uint16_t fan_ICE = 0;
        static constexpr uint16_t fan_AD = 1;
        static constexpr uint16_t fan_ballast = 2;

        // Discrete Inputs offset
        static constexpr uint16_t hasFault = 0;
        static constexpr uint16_t hasLimitViolations = 1;

    protected:
        QList<bool> coils_;
        QList<bool> discreteInputs_;
        QList<uint16_t> inputRegisters_;
        QList<uint16_t> holdingRegisters_;

    private:
        static constexpr size_t numberOfDiscreteInputs = 3;
        static constexpr size_t numberOfCoils = 3;
        static constexpr size_t numberOfInputRegisters = 50;
        static constexpr size_t numberOfHoldingRegisters = 44;

        bool doubleToInputReg(uint16_t offset, double data);
        double holdingRegToDouble(uint16_t offset);
        bool intToInputReg(uint16_t addr, uint64_t data);
        uint64_t holdingRegToInt(uint16_t offset);
        SimulationCommand holdingRegToCommand(uint16_t offset) const;
        SimulationRequest holdingRegToRequest(uint16_t offset) const;
        SimulationMode holdingRegToMode(uint16_t offset) const;
    };

}
#endif // REGISTERBANK_HPP
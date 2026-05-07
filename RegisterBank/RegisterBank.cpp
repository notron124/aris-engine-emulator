#include"RegisterBank.hpp"
#include <cstring>

namespace emulator::registerbank {

	ClientInputSnapshot RegisterBank::RegBankSendInfo() {
		ClientInputSnapshot clientSnapshot;
		ModelInputs registerBankStoredInputs;
		SimulationLimits limits;

		limits.T_cool_max = holdingRegToDouble(T_cool_max);
		limits.P_oil_min = holdingRegToDouble(P_oil_min);
		limits.P_oil_max = holdingRegToDouble(P_oil_max);
		limits.omega_ICE_max_prir = holdingRegToDouble(omega_ICE_max_prir);
		limits.omega_ICE_max_run = holdingRegToDouble(omega_ICE_max_run);
		limits.T_AD_max = holdingRegToDouble(T_AD_max);
		limits.T_ballast_max = holdingRegToDouble(T_ballast_max);

		registerBankStoredInputs.limits = limits;

		registerBankStoredInputs.f_AD = holdingRegToDouble(f_AD_Input);
		registerBankStoredInputs.M_AD_target = holdingRegToDouble(M_AD_target);

		registerBankStoredInputs.fan_ICE_enabled = coils_[fan_ICE];
		registerBankStoredInputs.fan_AD_enabled = coils_[fan_AD];
		registerBankStoredInputs.fan_ballast_enabled = coils_[fan_ballast];

		clientSnapshot.revision = holdingRegToInt(revision_h);
		clientSnapshot.command = holdingRegToCommand(simulationCommand);
		clientSnapshot.request = holdingRegToRequest(simulationRequest);
		registerBankStoredInputs.mode = holdingRegToMode(simulationMode);

		clientSnapshot.inputs = registerBankStoredInputs;

		return clientSnapshot;
	}
	bool RegisterBank::RegBankTakeInfo(const ModelOutputSnapshot& InputData) {
		intToInputReg(revision, InputData.revision);
		intToInputReg(sourceInputRevision, InputData.sourceInputRevision);

		if (InputData.outputs.has_value()) {
			ModelOutputs outputs = InputData.outputs.value();

			doubleToInputReg(T_cool, outputs.T_cool);
			doubleToInputReg(P_oil, outputs.P_oil);
			doubleToInputReg(omega_ICE_prir, outputs.omega_ICE_prir);
			doubleToInputReg(omega_ICE_run, outputs.omega_ICE_run);
			doubleToInputReg(T_AD, outputs.T_AD);
			doubleToInputReg(T_ballast, outputs.T_ballast);
			doubleToInputReg(M_AD, outputs.M_AD);
			doubleToInputReg(f_AD, outputs.f_AD);
		}
		inputRegisters_[state_ir] = static_cast<uint16_t>(InputData.state);
		inputRegisters_[faultCode] = static_cast<uint16_t>(InputData.diagnostics.faultCode);
		discreteInputs_[hasFault] = InputData.diagnostics.hasFault();
		discreteInputs_[hasLimitViolations] = InputData.runtimeDiagnostics.hasLimitViolations();

		intToInputReg(modelTime, static_cast<uint64_t>(InputData.modelTime.count()));

		uint64_t timestampMsec = InputData.timestampUtc.toMSecsSinceEpoch();
		intToInputReg(timestamp_ir, timestampMsec);

		return true;
	}
	//read 
	QList<bool> RegisterBank::readCoils(uint16_t startAddr, uint16_t count) const {
		if (startAddr + count > numberOfCoils) {
			return {};
		}
		return coils_.mid(startAddr, count);
	}

	QList<bool> RegisterBank::readDiscreteInputs(uint16_t startAddr, uint16_t count) const {
		if (startAddr + count > numberOfDiscreteInputs) {
			return {};
		}
		return discreteInputs_.mid(startAddr, count);
	}

	QList<uint16_t> RegisterBank::readInputRegisters(uint16_t startAddr, uint16_t count) const {
		if (startAddr + count > numberOfInputRegisters) {
			return {};
		}
		return inputRegisters_.mid(startAddr, count);
	}

	QList<uint16_t> RegisterBank::readHoldingRegisters(uint16_t startAddr, uint16_t count) const {
		if (startAddr + count > numberOfHoldingRegisters) {
			return {};
		}
		return holdingRegisters_.mid(startAddr, count);
	}
	// wite
	bool RegisterBank::writeCoil(uint16_t addr, bool value) {
		if (addr >= numberOfCoils) {
			return false;
		}
		coils_[addr] = value;
		return true;
	}

	bool RegisterBank::writeCoils(uint16_t startAddr, const QList<bool>& values) {
		if (startAddr + values.size() > numberOfCoils) {
			return false;
		}
		for (uint16_t i = startAddr; i < startAddr + values.size(); i++) {
			coils_[i] = values[i - startAddr];
		}
		return true;
	}

	bool RegisterBank::writeHoldingRegister(uint16_t addr, uint16_t value) {
		if (addr >= numberOfHoldingRegisters) {
			return false;
		}
		holdingRegisters_[addr] = value;
		return true;
	}

	bool RegisterBank::writeHoldingRegisters(uint16_t startAddr, const QList<uint16_t>& values) {
		if (startAddr + values.size() > numberOfHoldingRegisters) {
			return false;
		}
		for (uint16_t i = startAddr; i < startAddr + values.size(); i++) {
			holdingRegisters_[i] = values[i - startAddr];
		}
		return true;
	}

	bool RegisterBank::doubleToInputReg(uint16_t offset, double data) {
		memcpy(&inputRegisters_[offset], &data, sizeof(double));
		return true;
	}
	double RegisterBank::holdingRegToDouble(uint16_t offset) {
		double res;
		memcpy(&res, &holdingRegisters_[offset], sizeof(double));
		return res;
	}
	bool RegisterBank::intToInputReg(uint16_t addr, uint64_t data) {
		memcpy(&inputRegisters_[addr], &data, sizeof(uint64_t));
		return true;
	}
	uint64_t RegisterBank::holdingRegToInt(uint16_t offset) {
		uint64_t res;
		memcpy(&res, &holdingRegisters_[offset], sizeof(uint64_t));
		return res;
	}
	SimulationCommand RegisterBank::holdingRegToCommand(uint16_t offset) const {
		uint16_t val = holdingRegisters_[offset];
		if (val <= static_cast<uint16_t>(SimulationCommand::EmergencyStop))
			return static_cast<SimulationCommand>(val);
		return SimulationCommand::None;
	}

	SimulationRequest RegisterBank::holdingRegToRequest(uint16_t offset) const {
		uint16_t val = holdingRegisters_[offset];
		if (val <= static_cast<uint16_t>(SimulationRequest::StepAndRead))
			return static_cast<SimulationRequest>(val);
		return SimulationRequest::None;
	}

	SimulationMode RegisterBank::holdingRegToMode(uint16_t offset) const {
		uint16_t val = holdingRegisters_[offset];
		if (val <= static_cast<uint16_t>(SimulationMode::HotLoad))
			return static_cast<SimulationMode>(val);
		return SimulationMode::ColdRun;
	}
}
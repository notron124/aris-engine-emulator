struct incomingInfo {
	double currentDVSTemp = 98.2;
	int32_t AD_Freq = 1500;
};
struct outgoingInfo {
	uint32_t MaxTemp = 0;
	double DvsPMax = 0;
	bool ac_fan = 0;
};

class RegisterBank {
public:
	
	outgoingInfo RegBankSendInfo() {
		outgoingInfo OutputData;
		OutputData.MaxTemp = HoldingRegToInt(DVS_TEMP_MAX);
		OutputData.DvsPMax = HoldingRegToDouble(DVS_P_MAX);
		OutputData.ac_fan = Coils[FAN_AD];
		return OutputData;
	}
	bool RegBankTakeInfo(incomingInfo& InputData) const {
		IntToInputReg (FREQ, InputData.AD_freq);
		DoubleToInputReg(DVS_TEMP, InputData.currentDVSTemp);
		return 0;
	}

	std::vector<bool> readCoils(uint16_t startAddr, uint16_t count) const {
		std::vector<bool> response;
		for (uint16_t addr = startAddr; addr < startAddr + count; addr += 1) {
			response.push_back(Coils[addr]);
		}
		return response;
	}

	std::vector<bool> readDiscreteInputs(uint16_t startAddr, uint16_t count) const {
		std::vector<bool> response;
		for (uint16_t addr = startAddr; addr < startAddr + count; addr += 1) {
			response.push_back(DiscreteInputs[addr]);
		}
		return response;
	}

	std::vector<uint16_t> readInputRegisters(uint16_t startAddr, uint16_t count) const {
		std::vector<uint16_t> response;
		for (uint16_t addr = startAddr; addr < startAddr + count; addr += 1) {
			response.push_back(InputRegisters[addr]);
		}
		return response;
	}

	std::vector<uint16_t> readHoldingRegisters(uint16_t startAddr, uint16_t count) const {
		std::vector<uint16_t> response;
		for (uint16_t addr = startAddr; addr < startAddr + count; addr += 1) {
			response.push_back(HoldingRegisters[addr]);
		}
		return response;
	}
	// Запись
	bool writeCoil(uint16_t addr, bool value) {
		Coils[addr] = value;
		return true;
	}

	bool writeCoils(uint16_t startAddr, const std::vector<bool>& values) {
		for (uint16_t i = startAddr; i < startAddr + values.size(); i++) {
			Coils[i] = values[i - startAddr];
		}
		return true;
	}

	bool writeHoldingRegister(uint16_t addr, uint16_t value) {
		HoldingRegisters[addr] = value;
		return true;
	}

	bool writeHoldingRegisters(uint16_t startAddr, const std::vector<uint16_t>& values) {
		for (uint16_t i = startAddr; i < startAddr + values.size(); i++) {
			HoldingRegisters[i] = values[i - startAddr];
		}
		return true;
	}

	//serverGet offset list
	static constexpr uint16_t DVS_TEMP = 0;
	static constexpr uint16_t AD_TEMP = 4;
	static constexpr uint16_t BALL_TEMP = 8;
	static constexpr uint16_t DVS_P = 12;
	static constexpr uint16_t MOMENT = 16;
	static constexpr uint16_t FREQ = 30;

	//serverSetDuble offset list
	static constexpr uint16_t DVS_TEMP_MAX = 0;
	static constexpr uint16_t AD_TEMP_MAX = 4;
	static constexpr uint16_t BALL_TEMP_MAX = 8;
	static constexpr uint16_t DVS_P_MIN = 12;
	static constexpr uint16_t DVS_P_MAX = 16;
	static constexpr uint16_t FREQ_MAX_PRITIR = 20;
	static constexpr uint16_t FREQ_MAX_OBKAT = 24;
	
	//serverSetFlag offset list
	static constexpr uint16_t FAN_AD = 0;
	static constexpr uint16_t FAN_BALL = 1;

private:
	static constexpr uint32_t NumberOfDiscreteInputs = 1000;
	static constexpr uint32_t NumberOfCoils = 1000;
	static constexpr uint32_t NumberOfInputRegisters = 1000;
	static constexpr uint32_t NumberOfHoldingRegisters = 1000;
	//Поля для хранения данных
	std::array <bool, NumberOfDiscreteInputs> DiscreteInputs{};
	std::array <bool, NumberOfCoils> Coils{};
	std::array <uint16_t, NumberOfInputRegisters> InputRegisters{};
	std::array <uint16_t, NumberOfHoldingRegisters> HoldingRegisters{};

	bool DoubleToInputReg (uint16_t offset, double data) {
		memcpy(&InputRegisters[offset], &data, 8);
		return true;
	}
	double HoldingRegToDouble(uint16_t offset) {
		double res;
		memcpy(&res, &HoldingRegisters[offset], 8);
		return res;
	}
	bool IntToInputReg(uint16_t addr, int32_t data) {
		memcpy (&InputRegisters[addr], &data, 4);
		return true;
	}
	int32_t HoldingRegToInt(uint16_t offset) {
		int res;
		memcpy(&res, &HoldingRegisters[offset], 4);
		return res;
	}
};
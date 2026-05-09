#ifndef MODELADAPTER_HPP
#define MODELADAPTER_HPP

#include "SimulationTypes.hpp"
#include <chrono>

namespace emulator::simulation::model {

class ModelAdapter {
public:
    ~ModelAdapter() = default;
    
    // Жизненный цикл
    bool initialize();
    bool reset();
    
    // Управление
    bool setInputs(const ModelInputs& inputs);
    bool step(std::chrono::milliseconds modelTime, std::chrono::milliseconds dt);
    
    // Данные
    [[nodiscard]] ModelOutputs readOutputs() const;
    [[nodiscard]] bool isRunning() const;        // проверка на остановку пользователем
    [[nodiscard]] bool isEmergency() const;      // Быстрая проверка: авария?
    [[nodiscard]] DiagnosticsSnapshot diagnostics() const;
};

} 

#endif
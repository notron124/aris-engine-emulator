#ifndef MODELADAPTER_HPP
#define MODELADAPTER_HPP

#include "SimulationTypes.hpp"

namespace emulator::simulation::model {

/**
 * @class ModelAdapter
 * @brief Задаёт универсальный интерфейс доступа к расчётной модели
 * без привязки к конкретной реализации.
 *
 * @todo Лучше вынести в отдельный компонент вместе с реализацией,
 * всё равно в emulator::simulation достаточно неполного типа
 */
class ModelAdapter {
public:
    virtual ~ModelAdapter() = default;

    virtual bool initialize() = 0;
    virtual bool reset() = 0;
    virtual bool setInputs(const ModelInputs& inputs) = 0;
    virtual bool step(std::chrono::milliseconds modelTime,
                                    std::chrono::milliseconds dt) = 0;
    [[nodiscard]] virtual ModelOutputs readOutputs() const = 0;
    [[nodiscard]] virtual DiagnosticsSnapshot diagnostics() const = 0;
};

} // namespace emulator::simulation::model

#endif // MODELADAPTER_HPP

#ifndef SIMULATIONSNAPSHOTEXCHANGE_HPP
#define SIMULATIONSNAPSHOTEXCHANGE_HPP

#include "SimulationTypes.hpp"

namespace emulator::simulation {

/**
 * @class SimulationSnapshotExchange
 * @brief Описывает интерфейс атомарного чтения входных снимков и
 * публикации выходных снимков модели.
 *
 * @note Вызывает codec слоя RegisterBank для преобразования регистров в DTO.
 */
class SimulationSnapshotExchange {
public:
    virtual ~SimulationSnapshotExchange() = default;

    [[nodiscard]] virtual ClientInputSnapshot readClientInputSnapshot() = 0;
    virtual void publishModelOutputSnapshot(const ModelOutputSnapshot& snapshot) = 0;
};

} // namespace emulator::simulation

#endif // SIMULATIONSNAPSHOTEXCHANGE_HPP

#ifndef EXCHANGECONTRACT_HPP
#define EXCHANGECONTRACT_HPP

#include "simulation/contracts/ModelContract.hpp"
#include "simulation/contracts/SimulationDiagnostics.hpp"
#include "simulation/contracts/SimulationDomain.hpp"

#include <QDateTime>

#include <chrono>
#include <cstdint>
#include <optional>

namespace emulator::exchange {

/**
 * @brief Снимок входных данных, подготовленный из регистров backend'а.
 *
 * Этот DTO является границей между register-слоем и simulation-слоем:
 * register-слой преобразует сырые Modbus-регистры в этот тип, а
 * SimulationController принимает уже нормализованные значения.
 */
struct ClientInputSnapshot {
    /// @name Идентификация снимка
    /// @{
    ///< Служебный номер этого снимка.
    std::uint64_t revision = 0;
    /// @}

    ::emulator::simulation::SimulationCommand command =
        ::emulator::simulation::SimulationCommand::None;
    ::emulator::simulation::SimulationRequest request =
        ::emulator::simulation::SimulationRequest::None;
    std::optional<::emulator::model::ModelInputs> inputs;
};

/**
 * @brief Снимок выходных данных, подготовленный слоем simulation.
 *
 * Register-слой преобразует этот DTO обратно в значения регистров, не зная
 * деталей выполнения модели.
 */
struct ModelOutputSnapshot {
    /// @name Идентификация снимка
    /// @{
    /// Служебный номер этого снимка.
    std::uint64_t revision = 0;
    /// Служебный номер входного снимка,
    /// на основе которого был получен этот снимок.
    std::uint64_t sourceInputRevision = 0;
    /// Временная метка формирования снимка.
    QDateTime timestampUtc;
    /// @}

    ::emulator::simulation::SimulationState state =
        ::emulator::simulation::SimulationState::Stopped;
    std::optional<::emulator::model::ModelOutputs> outputs;
    /// Информация об ошибке симуляции.
    ::emulator::simulation::diagnostics::SimulationDiagnosticsSnapshot diagnostics;
    /// Дополнительные сведения о симуляции.
    ::emulator::simulation::diagnostics::RuntimeDiagnostics runtimeDiagnostics;
    std::chrono::milliseconds modelTime{0};
};

} // namespace emulator::exchange

#endif // EXCHANGECONTRACT_HPP

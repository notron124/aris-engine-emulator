#ifndef STEPONDEMANDSIMULATIONRUNNER_HPP
#define STEPONDEMANDSIMULATIONRUNNER_HPP

#include "runners/SimulationRunner.hpp"

namespace emulator::simulation::detail {

/**
 * @class StepOnDemandSimulationRunner
 * @brief Выполняет расчёт модели синхронно только по запросу StepAndRead.
 *
 * Модельное время продвигается только при обработке
 * SimulationRequest::StepAndRead:
 * 1. runner вычисляет интервал с прошлого расчётного снимка;
 * 2. делит его на SimulationConfig::integrationStep;
 * 3. последовательно вызывает ModelAdapter::step(...).
 *
 * @note Стратегия не запускает фоновый поток.
 */
class StepOnDemandSimulationRunner final : public SimulationRunnerBase {
public:
    /**
     * @brief Создаёт стратегию расчёта по запросу.
     *
     * @param controller Владелец стратегии, через него испускаются
     * наблюдательные сигналы.
     * @param modelAdapter Интерфейс расчётной модели.
     * @param config Режим и параметры расчётного шага.
     * @param clock Источник времени между входными снимками.
     */
    StepOnDemandSimulationRunner(
        SimulationController& controller,
        model::ModelAdapter& modelAdapter,
        SimulationConfig config,
        SimulationController::ClockFn clock);

    [[nodiscard]] std::optional<ModelOutputSnapshot> processSnapshot(
        const ClientInputSnapshot& inputSnapshot) override;

private:
    using TimePoint = SimulationController::TimePoint;

    /**
     * @brief Обрабатывает команду управления из входного снимка.
     *
     * @param snapshotTime Время получения снимка.
     */
    [[nodiscard]] std::optional<ModelOutputSnapshot> applyInputCommand(
        const ClientInputSnapshot& inputSnapshot,
        TimePoint snapshotTime);

    /**
     * @brief Выполняет расчёт заданной ширины шага и возвращает выходы.
     *
     * @param stepWidth Интервал обсчёта модели. Разбивается на внутренние шаги
     * SimulationConfig::integrationStep.
     */
    [[nodiscard]] std::optional<ModelOutputSnapshot> stepAndRead(
        const ClientInputSnapshot& inputSnapshot,
        std::chrono::milliseconds stepWidth);

    /**
     * @brief Запоминает время последнего расчётного снимка.
     */
    void markStepBoundary(TimePoint timePoint);

private:
    SimulationController::ClockFn clock_;

    bool hasStepBoundaryTime_ = false;
    TimePoint lastStepBoundaryTime_;
};

} // namespace emulator::simulation::detail

#endif // STEPONDEMANDSIMULATIONRUNNER_HPP

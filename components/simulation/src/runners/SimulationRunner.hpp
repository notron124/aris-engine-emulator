#ifndef SIMULATIONRUNNER_HPP
#define SIMULATIONRUNNER_HPP

#include "ModelAdapter.hpp"
#include "SimulationController.hpp"
#include "SimulationTypes.hpp"

#include <QString>

#include <chrono>
#include <optional>

namespace emulator::simulation::detail {

/**
 * @class SimulationRunner
 * @brief Задаёт внутренний интерфейс стратегии выполнения модели.
 *
 * SimulationController выбирает конкретную реализацию по
 * SimulationConfig::runMode и делегирует ей всю обработку входных снимков.
 *
 * @note SimulationRunner не является публичным API слоя simulation.
 */
class SimulationRunner {
public:
    virtual ~SimulationRunner() = default;

    /**
     * @brief Обрабатывает входной снимок в соответствии с выбранным режимом.
     *
     * Конкретная стратегия решает, нужно ли обработать команду управления,
     * вернуть текущее состояние, выполнить расчётный шаг или ничего не
     * публиковать.
     *
     * @return Выходной снимок, если после обработки есть данные для публикации;
     * std::nullopt, если публикация не требуется.
     */
    [[nodiscard]] virtual std::optional<ModelOutputSnapshot> processSnapshot(
        const ClientInputSnapshot& inputSnapshot) = 0;

    /**
     * @brief Возвращает текущее состояние симуляции внутри стратегии.
     */
    [[nodiscard]] virtual SimulationState state() const = 0;

    /**
     * @brief Проверяет, находится ли стратегия в состоянии
     * SimulationState::Running.
     */
    [[nodiscard]] virtual bool isRunning() const = 0;

    /**
     * @brief Возвращает текущее модельное время.
     */
    [[nodiscard]] virtual std::chrono::milliseconds modelTime() const = 0;

    /**
     * @brief Возвращает последний сформированный выходной снимок, если он есть.
     */
    [[nodiscard]] virtual std::optional<ModelOutputSnapshot> lastOutputSnapshot() const = 0;
};

/**
 * @class SimulationRunnerBase
 * @brief Содержит общую механику стратегий выполнения модели.
 *
 * Базовый класс хранит состояние симуляции, последнее модельное время,
 * текущие входы, последний выходной снимок и общие helper-методы.
 */
class SimulationRunnerBase : public SimulationRunner {
public:
    /**
     * @brief Создаёт базовую часть стратегии выполнения модели.
     *
     * @param controller Владелец стратегии, через него испускаются
     * наблюдательные сигналы.
     * @param modelAdapter Интерфейс расчётной модели. Объект должен жить дольше
     * runner.
     * @param config Режим и параметры расчётного шага.
     */
    SimulationRunnerBase(
        SimulationController& controller,
        model::ModelAdapter& modelAdapter,
        SimulationConfig config);

    [[nodiscard]] SimulationState state() const override;
    [[nodiscard]] bool isRunning() const override;
    [[nodiscard]] std::chrono::milliseconds modelTime() const override;
    [[nodiscard]] std::optional<ModelOutputSnapshot> lastOutputSnapshot() const override;

protected:
    /**
     * @brief Формирует fault-диагностику для внутренних ошибок runner/model API.
     *
     * Если модель уже вернула собственную fault-диагностику, она сохраняется.
     * Иначе создаётся внутренний fault с переданным сообщением.
     */
    [[nodiscard]] DiagnosticsSnapshot failureDiagnostics(
        const QString& message) const;

    /**
     * @brief Проверяет выходы модели на превышение лимитов симуляции.
     *
     * Возвращает диагностику модели, если ModelAdapter::diagnostics() уже
     * содержит fault. Иначе собирает все нарушения лимитов в один
     * DiagnosticsSnapshot.
     */
    [[nodiscard]] DiagnosticsSnapshot diagnosticsForOutputs(
        const ModelOutputs& outputs,
        const SimulationLimits& limits) const;

    /**
     * @brief Формирует снимок текущего состояния без продвижения модельного
     * времени.
     */
    [[nodiscard]] std::optional<ModelOutputSnapshot> readCurrentState(
        const ClientInputSnapshot& inputSnapshot);

    /**
     * @brief Создаёт ModelOutputSnapshot с новой ревизией.
     *
     * Метод не сохраняет снимок как последний. Для этого вызывается
     * `recordSnapshot(...)`.
     */
    [[nodiscard]] ModelOutputSnapshot makeOutputSnapshot(
        std::uint64_t sourceInputRevision,
        std::optional<ModelOutputs> outputs,
        const DiagnosticsSnapshot& diagnostics);

    /**
     * @brief Лениво инициализирует ModelAdapter.
     *
     * Если модель уже инициализирована, возвращает `true`. При ошибке
     * переводит симуляцию в SimulationState::Fault.
     */
    [[nodiscard]] bool ensureInitialized();

    /**
     * @brief Сбрасывает модель и локальное состояние runner.
     *
     * При ошибке сброса переводит симуляцию в SimulationState::Fault.
     */
    [[nodiscard]] bool resetModel();

    /**
     * @brief Запоминает новые входы модели из снимка, если они переданы.
     */
    void rememberInputs(const ClientInputSnapshot& inputSnapshot);

    /**
     * @brief Сохраняет последний выходной снимок и испускает
     * SimulationController::sig_outputProduced.
     */
    void recordSnapshot(const ModelOutputSnapshot& snapshot);

    /**
     * @brief Переводит симуляцию в SimulationState::Fault и испускает
     * SimulationController::sig_faultOccurred.
     */
    void enterFault(const DiagnosticsSnapshot& diagnostics);

    /**
     * @brief Меняет состояние симуляции и испускает
     * SimulationController::sig_stateChanged, если состояние изменилось.
     */
    void setState(SimulationState nextState);

protected:
    SimulationController& controller_;
    model::ModelAdapter& modelAdapter_;
    SimulationConfig config_;

    bool adapterInitialized_        = false;
    bool hasCurrentInputs_          = false;
    bool hasLastFaultDiagnostics_   = false;

    /**
     * @brief Накопленное модельное время.
     *
     * Показывает, до какой временной точки уже досчитана модель.
     *
     * @warning Это не ширина расчётного запроса.
     */
    std::chrono::milliseconds modelTime_{0};

    SimulationState state_ = SimulationState::Stopped;
    std::uint64_t outputRevision_ = 0;
    std::uint64_t lastInputRevision_ = 0;

    ModelInputs currentInputs_;
    std::optional<ModelOutputs> lastModelOutputs_;
    DiagnosticsSnapshot lastFaultDiagnostics_;
    std::optional<ModelOutputSnapshot> lastProducedSnapshot_;
};

} // namespace emulator::simulation::detail

#endif // SIMULATIONRUNNER_HPP

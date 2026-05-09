#ifndef SIMULATIONCONTROLLER_HPP
#define SIMULATIONCONTROLLER_HPP

#include "SimulationTypes.hpp"

#include <QObject>

#include <chrono>
#include <functional>
#include <memory>
#include <optional>

namespace emulator::simulation {

namespace model {
class ModelAdapter;
} // namespace model

namespace detail {
class SimulationRunner;
} // namespace detail

/**
 * @class SimulationController
 * @brief Управляет состоянием симуляции и расчётом модели
 * на основании входного снимка.
 */
class SimulationController : public QObject {
    Q_OBJECT

public:
    using Clock = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;
    using ClockFn = std::function<TimePoint()>;

    /**
     * @brief Создаёт контроллер симуляции с системными монотонными часами.
     *
     * @param modelAdapter Интерфейс расчётной модели. Объект должен жить дольше
     * контроллера.
     * @param config Режим выполнения и параметры расчётного шага.
     * @param parent Родительский QObject.
     */
    explicit SimulationController(
        model::ModelAdapter& modelAdapter,
        SimulationConfig config = continuous50Config,
        QObject* parent = nullptr);

    /**
     * @brief Создаёт контроллер симуляции с заданной функцией времени.
     *
     * Эта перегрузка нужна там, где важно явно управлять временем между
     * входными снимками.
     *
     * @param modelAdapter Интерфейс расчётной модели. Объект должен жить дольше
     * контроллера.
     * @param config Режим выполнения и параметры расчётного шага.
     * @param clock Функция получения текущего монотонного времени.
     * @param parent Родительский QObject.
     */
    explicit SimulationController(
        model::ModelAdapter& modelAdapter,
        SimulationConfig config,
        ClockFn clock,
        QObject* parent = nullptr);

    ~SimulationController() override;

    /**
     * @brief Обрабатывает входной снимок backend'а.
     *
     * Это основной публичный API контроллера. Управление симуляцией выполняется
     * только через поля ClientInputSnapshot:
     * - команды запуска/останова,
     * - запросы чтения состояния и входные параметры модели.
     *
     * @note Метод не работает с RegisterBank, RegisterCodec и Modbus-регистрами
     * напрямую. Эти детали остаются за пределами слоя simulation.
     *
     * @return Выходной снимок, если после обработки есть что публиковать в
     * регистровый слой; `std::nullopt`, если публикация не требуется.
     */
    [[nodiscard]] std::optional<ModelOutputSnapshot> processSnapshot(
        const ClientInputSnapshot& inputSnapshot);

    /**
     * @brief Возвращает текущее состояние симуляции.
     */
    [[nodiscard]] SimulationState state() const;

    /**
     * @brief Проверяет, находится ли симуляция в состоянии
     * SimulationState::Running.
     */
    [[nodiscard]] bool isRunning() const;

    /**
     * @brief Возвращает текущее модельное время.
     *
     * В SimulationRunMode::StepOnDemand значение увеличивается при обработке
     * SimulationRequest::StepAndRead.
     *
     * В SimulationRunMode::Continuous значение увеличивается
     * фоновым расчётным циклом.
     */
    [[nodiscard]] std::chrono::milliseconds modelTime() const;

    /**
     * @brief Возвращает последний сформированный выходной снимок, если он есть.
     */
    [[nodiscard]] std::optional<ModelOutputSnapshot> lastOutputSnapshot() const;

signals:
    /**
     * @brief Сигнал изменения состояния симуляции.
     *
     * Испускается внутренней стратегией выполнения, когда состояние меняется
     * между SimulationState::Stopped, SimulationState::Running и
     * SimulationState::Fault.
     *
     * Сигнал является наблюдательным: команды управления не передаются через
     * сигнал, а приходят только через ClientInputSnapshot.
     *
     * @note SimulationState зарегистрирован как метатип Qt.
     */
    void sig_stateChanged(
        emulator::simulation::SimulationState state);

    /**
     * @brief Сигнал возникновения fault-состояния.
     *
     * Испускается, когда контроллер или стратегия выполнения переводит
     * симуляцию в SimulationState::Fault. Например, при ошибке модели,
     * превышении лимитов или неподдерживаемом SimulationRunMode.
     *
     * @note DiagnosticsSnapshot зарегистрирован как метатип Qt.
     */
    void sig_faultOccurred(
        emulator::simulation::DiagnosticsSnapshot diagnostics);

    /**
     * @brief Сигнал о формировании выходного снимка контроллером.
     *
     * Испускается после того, как стратегия выполнения сформировала и запомнила
     * ModelOutputSnapshot.
     *
     * Сигнал является наблюдательным и не публикует данные в RegisterBank:
     * публикацию выполняет SimulationBackendBridge через
     * SimulationSnapshotExchange.
     *
     * @param outputRevision Ревизия сформированного ModelOutputSnapshot.
     * @param sourceInputRevision Ревизия входного ClientInputSnapshot, на
     * основании которого был сформирован выходной снимок.
     */
    void sig_outputProduced(
        std::uint64_t outputRevision, std::uint64_t sourceInputRevision);

private:
    /// Стратегия симуляции
    std::unique_ptr<detail::SimulationRunner> runner_;
};

} // namespace emulator::simulation

#endif // SIMULATIONCONTROLLER_HPP

#ifndef CONTINUOUSSIMULATIONRUNNER_HPP
#define CONTINUOUSSIMULATIONRUNNER_HPP

#include "runners/SimulationRunner.hpp"

#include <condition_variable>
#include <mutex>
#include <thread>

namespace emulator::simulation::detail {

/**
 * @class ContinuousSimulationRunner
 * @brief Выполняет непрерывный расчёт модели в отдельном потоке.
 *
 * После SimulationCommand::Start стратегия запускает worker один раз и
 * выполняет модель с периодом SimulationConfig::integrationStep, пока
 * симуляция находится в состоянии SimulationState::Running.
 *
 * Входные снимки не выполняют отдельный расчётный шаг. Они обновляют входы,
 * обрабатывают команды или возвращают текущее состояние фонового расчёта.
 */
class ContinuousSimulationRunner final : public SimulationRunnerBase {
public:
    /**
     * @brief Создаёт стратегию непрерывного фонового расчёта.
     *
     * @param controller Владелец стратегии, через него испускаются
     * наблюдательные сигналы.
     * @param modelAdapter Интерфейс расчётной модели.
     * @param config Режим и параметры расчётного шага.
     */
    ContinuousSimulationRunner(SimulationController& controller,
                               model::ModelAdapter& modelAdapter,
                               SimulationConfig config);

    /**
     * @details Останавливает worker-поток перед уничтожением стратегии.
     */
    ~ContinuousSimulationRunner() override;

    [[nodiscard]] std::optional<exchange::ModelOutputSnapshot> processSnapshot(
        const exchange::ClientInputSnapshot& inputSnapshot) override;
    [[nodiscard]] SimulationState state() const override;
    [[nodiscard]] bool isRunning() const override;
    [[nodiscard]] std::chrono::milliseconds modelTime() const override;
    [[nodiscard]] std::optional<exchange::ModelOutputSnapshot> lastOutputSnapshot() const override;

private:
    /**
     * @brief Обрабатывает команду управления из входного снимка.
     */
    [[nodiscard]] std::optional<exchange::ModelOutputSnapshot> applyInputCommand(
        const exchange::ClientInputSnapshot& inputSnapshot);

    /**
     * @brief Запускает worker-поток, если он ещё не был создан.
     */
    void startWorker();

    /**
     * @brief Запрашивает останов worker-потока и ждёт его завершения.
     */
    void stopWorker();

    /**
     * @brief Основной цикл фонового расчёта модели.
     *
     * Цикл ждёт состояние Running, применяет текущие входы, выполняет
     * ModelAdapter::step(...), читает выходы и проверяет лимиты.
     */
    void workerLoop();

private:
    mutable std::mutex mutex_;
    std::condition_variable workerCv_;
    std::thread workerThread_;
    bool workerStopRequested_ = false;
    bool workerStarted_ = false;
};

} // namespace emulator::simulation::detail

#endif // CONTINUOUSSIMULATIONRUNNER_HPP

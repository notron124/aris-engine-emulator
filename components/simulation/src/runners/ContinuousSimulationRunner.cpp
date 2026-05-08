#include "runners/ContinuousSimulationRunner.hpp"

#include <utility>

namespace emulator::simulation::detail {

ContinuousSimulationRunner::ContinuousSimulationRunner(
    SimulationController& controller,
    model::ModelAdapter& modelAdapter,
    SimulationConfig config)
    : SimulationRunnerBase(controller, modelAdapter, std::move(config))
{
}

ContinuousSimulationRunner::~ContinuousSimulationRunner()
{
    stopWorker();
}

std::optional<ModelOutputSnapshot>
ContinuousSimulationRunner::processSnapshot(
    const ClientInputSnapshot& inputSnapshot)
{
    std::optional<ModelOutputSnapshot> result;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        lastInputRevision_ = inputSnapshot.revision;
        rememberInputs(inputSnapshot);

        if (const auto commandResult = applyInputCommand(inputSnapshot)) {
            result = commandResult;
        } else {
            switch (inputSnapshot.request) {
            case SimulationRequest::None:
                break;

            case SimulationRequest::ReadCurrentState:
            case SimulationRequest::StepAndRead:
                result = readCurrentState(inputSnapshot);
                break;
            }
        }

        if (result.has_value()) {
            recordSnapshot(*result);
        }
    }

    workerCv_.notify_all();
    return result;
}

SimulationState ContinuousSimulationRunner::state() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return SimulationRunnerBase::state();
}

bool ContinuousSimulationRunner::isRunning() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return SimulationRunnerBase::isRunning();
}

std::chrono::milliseconds ContinuousSimulationRunner::modelTime() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return SimulationRunnerBase::modelTime();
}

std::optional<ModelOutputSnapshot>
ContinuousSimulationRunner::lastOutputSnapshot() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return SimulationRunnerBase::lastOutputSnapshot();
}

std::optional<ModelOutputSnapshot>
ContinuousSimulationRunner::applyInputCommand(
    const ClientInputSnapshot& inputSnapshot)
{
    switch (inputSnapshot.command) {
    case SimulationCommand::None:
        return std::nullopt;

    case SimulationCommand::Start: {
        auto result = startModel(inputSnapshot);
        if (state_ == SimulationState::Running) {
            startWorker();
        }
        return result;
    }

    case SimulationCommand::Stop:
        return stopModel(inputSnapshot);

    case SimulationCommand::Reset:
        return resetModelAndMakeSnapshot(inputSnapshot);

    case SimulationCommand::EmergencyStop:
        return emergencyStop(inputSnapshot);
    }

    return std::nullopt;
}

void
ContinuousSimulationRunner::startWorker()
{
    if (workerStarted_) {
        return;
    }

    workerStopRequested_ = false;
    workerStarted_ = true;
    workerThread_ = std::thread([this] {
        workerLoop();
    });
}

void
ContinuousSimulationRunner::stopWorker()
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!workerStarted_) {
            return;
        }
        workerStopRequested_ = true;
    }

    workerCv_.notify_all();

    if (workerThread_.joinable()) {
        workerThread_.join();
    }

    std::lock_guard<std::mutex> lock(mutex_);
    workerStarted_ = false;
    workerStopRequested_ = false;
}

void
ContinuousSimulationRunner::workerLoop()
{
    while (true) {
        const auto dt = normalizedIntegrationStep();

        {
            std::unique_lock<std::mutex> lock(mutex_);

            // Спит, пока нет команды остановки и нет данных для расчёта.
            workerCv_.wait(lock, [this] {
                return workerStopRequested_
                    || (state_ == SimulationState::Running && hasCurrentInputs_);
            });

            // Если поток проснулся, потому что попросили остановиться
            if (workerStopRequested_) {
                return;
            }

            if (!ensureInitialized()) {
                continue;
            }

            if (!modelAdapter_.setInputs(currentInputs_)) {
                const auto diagnostics = failureDiagnostics(QStringLiteral("Model adapter rejected inputs"));
                enterFault(diagnostics);
                recordSnapshot(makeOutputSnapshot(lastInputRevision_, lastModelOutputs_, diagnostics));
                continue;
            }

            /// @todo Contiunous-режим сейчас не компенсирует время выполнения
            /// ModelAdapter::step(...). После каждого шага поток дополнительно
            /// ждёт dt, поэтому фактический период цикла равен примерно
            /// stepDuration + dt.
            ///
            /// @code
            /// step()
            /// sleep(dt)
            /// step()
            /// sleep(dt)
            /// @endcode
            ///
            /// Если stepDuration заметен относительно dt,
            /// модельное время начинает отставать от реального.
            ///
            /// Нужно перейти на планирование по абсолютному времени через
            /// wait_until(nextTick) и добавить диагностику overrun.
            ///
            /// @code
            /// dt = 50 мс
            ///
            /// tick #1 стартовал в 0 мс
            /// step занял 80 мс
            /// следующий tick должен был быть в 50 мс
            /// но сейчас уже 80 мс
            /// опоздали на 30 мс - overrun
            /// @endcode

            if (!modelAdapter_.step(modelTime_, dt)) {
                const auto diagnostics = failureDiagnostics(QStringLiteral("Model adapter step failed"));
                enterFault(diagnostics);
                recordSnapshot(makeOutputSnapshot(lastInputRevision_, modelAdapter_.readOutputs(), diagnostics));
                continue;
            }

            modelTime_ += dt;

            const auto outputs = modelAdapter_.readOutputs();
            lastModelOutputs_ = outputs;

            const auto runtimeDiagnostics = runtimeDiagnosticsForOutputs(
                outputs,
                currentInputs_.limits);
            lastRuntimeDiagnostics_ = runtimeDiagnostics;

            const auto diagnostics = modelAdapter_.diagnostics();
            if (diagnostics.hasFault()) {
                enterFault(diagnostics);
                recordSnapshot(makeOutputSnapshot(lastInputRevision_,
                                                  outputs,
                                                  diagnostics,
                                                  runtimeDiagnostics));
                continue;
            }

            if (shouldStopOnLimitViolation(runtimeDiagnostics)) {
                const auto limitDiagnostics =
                    limitViolationFaultDiagnostics(runtimeDiagnostics);
                enterFault(limitDiagnostics);
                recordSnapshot(makeOutputSnapshot(lastInputRevision_,
                                                  outputs,
                                                  limitDiagnostics,
                                                  runtimeDiagnostics));
                continue;
            }
        }

        // После шага модели ждёт dt, но если раньше пришла команда сброса,
        // сразу просыпается
        std::unique_lock<std::mutex> sleepLock(mutex_);
        workerCv_.wait_for(sleepLock, dt, [this] {
            return workerStopRequested_ || state_ != SimulationState::Running;
        });

        if (workerStopRequested_) {
            return;
        }
    }
}

} // namespace emulator::simulation::detail

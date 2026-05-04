#include "SimulationController.hpp"

#include "runners/ContinuousSimulationRunner.hpp"
#include "runners/StepOnDemandSimulationRunner.hpp"

#include <memory>
#include <utility>

namespace emulator::simulation {

namespace {

constexpr int UnsupportedRunModeFaultCode = 666;

DiagnosticsSnapshot
unsupportedRunModeDiagnostics()
{
    DiagnosticsSnapshot diagnostics {
        .hasFault = true,
        .faultCode = UnsupportedRunModeFaultCode,
        .message = QStringLiteral("Unsupported simulation run mode")
    };
    return diagnostics;
}

std::unique_ptr<detail::SimulationRunner>
createRunner(
    SimulationController& controller,
    model::ModelAdapter& modelAdapter,
    SimulationConfig config,
    SimulationController::ClockFn clock)
{
    switch (config.runMode) {
    case SimulationRunMode::StepOnDemand:
        return std::make_unique<detail::StepOnDemandSimulationRunner>(
            controller,
            modelAdapter,
            config,
            std::move(clock));

    case SimulationRunMode::Continuous:
        return std::make_unique<detail::ContinuousSimulationRunner>(
            controller,
            modelAdapter,
            config);
    }

    return nullptr;
}

} // namespace

SimulationController::SimulationController(
    model::ModelAdapter& modelAdapter,
    SimulationConfig config,
    QObject* parent)
    : SimulationController(modelAdapter,
                           std::move(config),
                           [] { return Clock::now(); },
                           parent)
{
}

SimulationController::SimulationController(
    model::ModelAdapter& modelAdapter,
    SimulationConfig config,
    ClockFn clock,
    QObject* parent)
    : QObject(parent)
    , runner_(createRunner(*this, modelAdapter, std::move(config), std::move(clock)))
{
    qRegisterMetaType<SimulationState>("emulator::simulation::SimulationState");
    qRegisterMetaType<DiagnosticsSnapshot>("emulator::simulation::DiagnosticsSnapshot");
}

SimulationController::~SimulationController() = default;

std::optional<ModelOutputSnapshot>
SimulationController::processSnapshot(const ClientInputSnapshot& inputSnapshot)
{
    if (!runner_) {
        emit sig_faultOccurred(unsupportedRunModeDiagnostics());
        return std::nullopt;
    }

    return runner_->processSnapshot(inputSnapshot);
}

SimulationState
SimulationController::state() const
{
    if (!runner_) {
        return SimulationState::Fault;
    }

    return runner_->state();
}

bool
SimulationController::isRunning() const
{
    if (!runner_) {
        return false;
    }

    return runner_->isRunning();
}

std::chrono::milliseconds
SimulationController::modelTime() const
{
    if (!runner_) {
        return std::chrono::milliseconds{0};
    }

    return runner_->modelTime();
}

std::optional<ModelOutputSnapshot>
SimulationController::lastOutputSnapshot() const
{
    if (!runner_) {
        return std::nullopt;
    }

    return runner_->lastOutputSnapshot();
}

} // namespace emulator::simulation

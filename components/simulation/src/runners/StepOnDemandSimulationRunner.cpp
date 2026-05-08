#include "runners/StepOnDemandSimulationRunner.hpp"

#include <algorithm>
#include <utility>

namespace emulator::simulation::detail {

StepOnDemandSimulationRunner::StepOnDemandSimulationRunner(
    SimulationController& controller,
    model::ModelAdapter& modelAdapter,
    SimulationConfig config,
    SimulationController::ClockFn clock)
    : SimulationRunnerBase(controller, modelAdapter, std::move(config))
    , clock_(std::move(clock))
{
}

std::optional<ModelOutputSnapshot>
StepOnDemandSimulationRunner::processSnapshot(
    const ClientInputSnapshot& inputSnapshot)
{
    const auto now = clock_();
    const auto hadInputsBefore = hasCurrentInputs_;
    lastInputRevision_ = inputSnapshot.revision;
    rememberInputs(inputSnapshot);
    if (!hadInputsBefore
        && hasCurrentInputs_
        && state_ == SimulationState::Running
        && !hasStepBoundaryTime_) {
        markStepBoundary(now);
    }

    const auto recordAndReturn = [this](std::optional<ModelOutputSnapshot> snapshot) {
        if (snapshot.has_value()) {
            recordSnapshot(*snapshot);
        }
        return snapshot;
    };

    if (const auto commandResult = applyInputCommand(inputSnapshot, now)) {
        return recordAndReturn(commandResult);
    }

    switch (inputSnapshot.request) {
    case SimulationRequest::None:
        return std::nullopt;

    case SimulationRequest::ReadCurrentState:
        return recordAndReturn(readCurrentState(inputSnapshot));

    case SimulationRequest::StepAndRead:
        if (state_ != SimulationState::Running) {
            return recordAndReturn(readCurrentState(inputSnapshot));
        }

        if (!hasCurrentInputs_) {
            return recordAndReturn(readCurrentState(inputSnapshot));
        }

        if (!hasStepBoundaryTime_) {
            markStepBoundary(now);
            return recordAndReturn(readCurrentState(inputSnapshot));
        }

        const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - lastStepBoundaryTime_);
        markStepBoundary(now);

        if (elapsed <= std::chrono::milliseconds::zero()) {
            return recordAndReturn(readCurrentState(inputSnapshot));
        }

        return recordAndReturn(stepAndRead(inputSnapshot, elapsed));
    }

    return std::nullopt;
}

std::optional<ModelOutputSnapshot>
StepOnDemandSimulationRunner::applyInputCommand(
    const ClientInputSnapshot& inputSnapshot,
    TimePoint snapshotTime)
{
    switch (inputSnapshot.command) {
    case SimulationCommand::None:
        return std::nullopt;

    case SimulationCommand::Start: {
        auto result = startModel(inputSnapshot);
        if (state_ == SimulationState::Running && hasCurrentInputs_) {
            markStepBoundary(snapshotTime);
        }
        return result;
    }

    case SimulationCommand::Stop:
        hasStepBoundaryTime_ = false;
        return stopModel(inputSnapshot);

    case SimulationCommand::Reset:
        hasStepBoundaryTime_ = false;
        return resetModelAndMakeSnapshot(inputSnapshot);

    case SimulationCommand::EmergencyStop:
        return emergencyStop(inputSnapshot);
    }

    return std::nullopt;
}

std::optional<ModelOutputSnapshot>
StepOnDemandSimulationRunner::stepAndRead(
    const ClientInputSnapshot& inputSnapshot,
    std::chrono::milliseconds stepWidth)
{
    if (stepWidth <= std::chrono::milliseconds::zero()) {
        return readCurrentState(inputSnapshot);
    }

    if (!ensureInitialized()) {
        const auto diagnostics = hasLastFaultDiagnostics_
            ? lastFaultDiagnostics_
            : failureDiagnostics(QStringLiteral("Model adapter initialization failed"));
        return makeOutputSnapshot(inputSnapshot.revision, lastModelOutputs_, diagnostics);
    }

    if (!hasCurrentInputs_) {
        return readCurrentState(inputSnapshot);
    }

    if (!modelAdapter_.setInputs(currentInputs_)) {
        const auto diagnostics =
            failureDiagnostics(QStringLiteral("Model adapter rejected inputs"));
        enterFault(diagnostics);
        return makeOutputSnapshot(inputSnapshot.revision, lastModelOutputs_, diagnostics);
    }

    auto remaining = stepWidth;
    const auto integrationStep = normalizedIntegrationStep();
    while (remaining > std::chrono::milliseconds::zero()) {
        const auto dt = std::min(remaining, integrationStep);

        if (!modelAdapter_.step(modelTime_, dt)) {
            const auto diagnostics =
                failureDiagnostics(QStringLiteral("Model adapter step failed"));
            enterFault(diagnostics);
            return makeOutputSnapshot(inputSnapshot.revision,
                                      modelAdapter_.readOutputs(),
                                      diagnostics);
        }

        modelTime_ += dt;
        remaining -= dt;
    }

    const auto outputs = modelAdapter_.readOutputs();
    const auto runtimeDiagnostics =
        runtimeDiagnosticsForOutputs(outputs, currentInputs_.limits);

    const auto diagnostics = modelAdapter_.diagnostics();
    if (diagnostics.hasFault()) {
        enterFault(diagnostics);
        return makeOutputSnapshot(inputSnapshot.revision,
                                  outputs,
                                  diagnostics,
                                  runtimeDiagnostics);
    }

    if (shouldStopOnLimitViolation(runtimeDiagnostics)) {
        const auto limitDiagnostics =
            limitViolationFaultDiagnostics(runtimeDiagnostics);
        enterFault(limitDiagnostics);
        return makeOutputSnapshot(inputSnapshot.revision,
                                  outputs,
                                  limitDiagnostics,
                                  runtimeDiagnostics);
    }

    return makeOutputSnapshot(inputSnapshot.revision,
                              outputs,
                              diagnostics,
                              runtimeDiagnostics);
}

void StepOnDemandSimulationRunner::markStepBoundary(TimePoint timePoint)
{
    lastStepBoundaryTime_ = timePoint;
    hasStepBoundaryTime_ = true;
}

} // namespace emulator::simulation::detail

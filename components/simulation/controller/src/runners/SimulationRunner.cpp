#include "runners/SimulationRunner.hpp"

#include <QStringList>

#include <utility>

namespace emulator::simulation::detail {

SimulationRunnerBase::SimulationRunnerBase(
    SimulationController& controller,
    model::ModelAdapter& modelAdapter,
    SimulationConfig config)
    : controller_(controller)
    , modelAdapter_(modelAdapter)
    , config_(std::move(config))
{
}

SimulationState SimulationRunnerBase::state() const
{
    return state_;
}

bool SimulationRunnerBase::isRunning() const
{
    return state_ == SimulationState::Running;
}

std::chrono::milliseconds SimulationRunnerBase::modelTime() const
{
    return modelTime_;
}

std::optional<exchange::ModelOutputSnapshot> SimulationRunnerBase::lastOutputSnapshot() const
{
    return lastProducedSnapshot_;
}

std::chrono::milliseconds
SimulationRunnerBase::normalizedIntegrationStep() const
{
    if (config_.integrationStep > std::chrono::milliseconds::zero()) {
        return config_.integrationStep;
    }

    return std::chrono::milliseconds{1};
}

std::optional<exchange::ModelOutputSnapshot>
SimulationRunnerBase::startModel(const exchange::ClientInputSnapshot& inputSnapshot)
{
    if (state_ == SimulationState::Fault) {
        const auto diagnostics = hasLastFaultDiagnostics_
            ? lastFaultDiagnostics_
            : failureDiagnostics(QStringLiteral("Simulation is in fault state"));
        return makeOutputSnapshot(inputSnapshot.revision, lastModelOutputs_, diagnostics);
    }

    if (!ensureInitialized()) {
        const auto diagnostics = hasLastFaultDiagnostics_
            ? lastFaultDiagnostics_
            : failureDiagnostics(QStringLiteral("Model adapter initialization failed"));
        return makeOutputSnapshot(inputSnapshot.revision, lastModelOutputs_, diagnostics);
    }

    setState(SimulationState::Running);
    return makeOutputSnapshot(inputSnapshot.revision,
                              std::nullopt,
                              modelAdapter_.diagnostics());
}

std::optional<exchange::ModelOutputSnapshot>
SimulationRunnerBase::stopModel(const exchange::ClientInputSnapshot& inputSnapshot)
{
    setState(SimulationState::Stopped);
    return readCurrentState(inputSnapshot);
}

std::optional<exchange::ModelOutputSnapshot>
SimulationRunnerBase::resetModelAndMakeSnapshot(
    const exchange::ClientInputSnapshot& inputSnapshot)
{
    if (!resetModel()) {
        const auto diagnostics = hasLastFaultDiagnostics_
            ? lastFaultDiagnostics_
            : failureDiagnostics(QStringLiteral("Model adapter reset failed"));
        return makeOutputSnapshot(inputSnapshot.revision, lastModelOutputs_, diagnostics);
    }

    return makeOutputSnapshot(inputSnapshot.revision, std::nullopt, DiagnosticsSnapshot{});
}

std::optional<exchange::ModelOutputSnapshot>
SimulationRunnerBase::emergencyStop(const exchange::ClientInputSnapshot& inputSnapshot)
{
    DiagnosticsSnapshot diagnostics;
    diagnostics.faultCode = SimulationFaultCode::EmergencyStop;
    diagnostics.message = QStringLiteral("Emergency stop requested");
    enterFault(diagnostics);
    return makeOutputSnapshot(inputSnapshot.revision, lastModelOutputs_, diagnostics);
}

std::optional<exchange::ModelOutputSnapshot>
SimulationRunnerBase::readCurrentState(const exchange::ClientInputSnapshot& inputSnapshot)
{
    const auto diagnostics = state_ == SimulationState::Fault && hasLastFaultDiagnostics_
        ? lastFaultDiagnostics_
        : (adapterInitialized_ ? modelAdapter_.diagnostics() : DiagnosticsSnapshot{});

    return makeOutputSnapshot(inputSnapshot.revision,
                              lastModelOutputs_,
                              diagnostics,
                              lastRuntimeDiagnostics_);
}

exchange::ModelOutputSnapshot
SimulationRunnerBase::makeOutputSnapshot(
    std::uint64_t sourceInputRevision,
    std::optional<ModelOutputs> outputs,
    const DiagnosticsSnapshot& diagnostics,
    RuntimeDiagnostics runtimeDiagnostics)
{
    exchange::ModelOutputSnapshot snapshot;
    snapshot.revision = ++outputRevision_;
    snapshot.sourceInputRevision = sourceInputRevision;
    snapshot.state = state_;
    snapshot.outputs = std::move(outputs);
    snapshot.diagnostics = diagnostics;
    snapshot.runtimeDiagnostics = std::move(runtimeDiagnostics);
    snapshot.modelTime = modelTime_;
    snapshot.timestampUtc = QDateTime::currentDateTimeUtc();
    return snapshot;
}

DiagnosticsSnapshot
SimulationRunnerBase::failureDiagnostics(
    const QString& message) const
{
    auto diagnostics =
        adapterInitialized_ ? modelAdapter_.diagnostics() : DiagnosticsSnapshot{};
    if (!diagnostics.hasFault()) {
        diagnostics.faultCode = SimulationFaultCode::InternalError;
        diagnostics.message = message;
    }

    return diagnostics;
}

RuntimeDiagnostics
SimulationRunnerBase::runtimeDiagnosticsForOutputs(
    const ModelOutputs& outputs,
    const Limits &limits) const
{
    RuntimeDiagnostics diagnostics;
    const auto appendViolation = [&diagnostics](
                                     const QString& name,
                                     double value,
                                     const QString& relation,
                                     double limit) {
        diagnostics.limitViolations.push_back(LimitViolation {
            name,
            value,
            relation,
            limit
        });
    };

    /// @todo Архитектурно негибкая логика:
    /// у модели могжет поменяться состав контролируемых параметров

    if (outputs.T_cool > limits.T_cool_max) {
        appendViolation(QStringLiteral("T_cool"),
                        outputs.T_cool,
                        QStringLiteral(">"),
                        limits.T_cool_max);
    }
    if (outputs.P_oil < limits.P_oil_min) {
        appendViolation(QStringLiteral("P_oil"),
                        outputs.P_oil,
                        QStringLiteral("<"),
                        limits.P_oil_min);
    }
    if (outputs.P_oil > limits.P_oil_max) {
        appendViolation(QStringLiteral("P_oil"),
                        outputs.P_oil,
                        QStringLiteral(">"),
                        limits.P_oil_max);
    }
    if (outputs.omega_ICE_prir > limits.omega_ICE_max_prir) {
        appendViolation(QStringLiteral("omega_ICE_prir"),
                        outputs.omega_ICE_prir,
                        QStringLiteral(">"),
                        limits.omega_ICE_max_prir);
    }
    if (outputs.omega_ICE_run > limits.omega_ICE_max_run) {
        appendViolation(QStringLiteral("omega_ICE_run"),
                        outputs.omega_ICE_run,
                        QStringLiteral(">"),
                        limits.omega_ICE_max_run);
    }
    if (outputs.T_AD > limits.T_AD_max) {
        appendViolation(QStringLiteral("T_AD"),
                        outputs.T_AD,
                        QStringLiteral(">"),
                        limits.T_AD_max);
    }
    if (outputs.T_ballast > limits.T_ballast_max) {
        appendViolation(QStringLiteral("T_ballast"),
                        outputs.T_ballast,
                        QStringLiteral(">"),
                        limits.T_ballast_max);
    }

    return diagnostics;
}

DiagnosticsSnapshot
SimulationRunnerBase::limitViolationFaultDiagnostics(
    const RuntimeDiagnostics& runtimeDiagnostics) const
{
    DiagnosticsSnapshot diagnostics;
    if (!runtimeDiagnostics.hasLimitViolations()) {
        return diagnostics;
    }

    QStringList violations;
    for (const auto& violation : runtimeDiagnostics.limitViolations) {
        violations.push_back(QStringLiteral("%1=%2 %3 %4")
                                 .arg(violation.parameter)
                                 .arg(violation.value)
                                 .arg(violation.relation)
                                 .arg(violation.limit));
    }

    diagnostics.faultCode = SimulationFaultCode::SimulationLimitsExceeded;
    diagnostics.message = QStringLiteral("Simulation limits exceeded: %1")
                              .arg(violations.join(QStringLiteral("; ")));
    return diagnostics;
}

bool
SimulationRunnerBase::shouldStopOnLimitViolation(
    const RuntimeDiagnostics& runtimeDiagnostics) const
{
    return config_.limitViolationAction == LimitViolationAction::StopSimulation
        && runtimeDiagnostics.hasLimitViolations();
}

bool SimulationRunnerBase::ensureInitialized()
{
    if (adapterInitialized_) {
        return true;
    }

    if (!modelAdapter_.initialize()) {
        enterFault(failureDiagnostics(QStringLiteral("Model adapter initialization failed")));
        return false;
    }

    adapterInitialized_ = true;
    return true;
}

bool SimulationRunnerBase::resetModel()
{
    if (adapterInitialized_ && !modelAdapter_.reset()) {
        enterFault(failureDiagnostics(QStringLiteral("Model adapter reset failed")));
        return false;
    }

    adapterInitialized_ = false;
    hasCurrentInputs_ = false;
    hasLastFaultDiagnostics_ = false;
    currentInputs_ = ModelInputs{};
    lastModelOutputs_.reset();
    lastRuntimeDiagnostics_ = RuntimeDiagnostics{};
    lastProducedSnapshot_.reset();
    modelTime_ = std::chrono::milliseconds{0};
    setState(SimulationState::Stopped);
    return true;
}

void SimulationRunnerBase::rememberInputs(
    const exchange::ClientInputSnapshot& inputSnapshot)
{
    if (!inputSnapshot.inputs.has_value()) {
        return;
    }

    currentInputs_ = *inputSnapshot.inputs;
    hasCurrentInputs_ = true;
}

void SimulationRunnerBase::recordSnapshot(
    const exchange::ModelOutputSnapshot& snapshot)
{
    lastProducedSnapshot_ = snapshot;
    lastRuntimeDiagnostics_ = snapshot.runtimeDiagnostics;
    if (snapshot.outputs.has_value()) {
        lastModelOutputs_ = snapshot.outputs;
    }

    emit controller_.sig_outputProduced(snapshot.revision, snapshot.sourceInputRevision);
}

void SimulationRunnerBase::enterFault(const DiagnosticsSnapshot& diagnostics)
{
    lastFaultDiagnostics_ = diagnostics;
    hasLastFaultDiagnostics_ = true;
    setState(SimulationState::Fault);
    emit controller_.sig_faultOccurred(diagnostics);
}

void SimulationRunnerBase::setState(SimulationState nextState)
{
    if (state_ == nextState) {
        return;
    }

    state_ = nextState;
    emit controller_.sig_stateChanged(state_);
}

} // namespace emulator::simulation::detail

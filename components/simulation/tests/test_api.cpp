#include <QtTest>
#include <QSignalSpy>

#include <cstdint>
#include <vector>

#include "ModelAdapter.hpp"
#include "simulation/exchange/SimulationBackendBridge.hpp"
#include "simulation/controller/SimulationController.hpp"
#include "simulation/exchange/SimulationSnapshotExchange.hpp"

namespace {
using namespace emulator::simulation;
using namespace emulator::model;
using emulator::exchange::ClientInputSnapshot;
using emulator::exchange::ModelOutputSnapshot;

constexpr auto testModelFaultCode =
    emulator::model::diagnostics::ModelFaultCode::ModelFault;

constexpr Limits testLimits {
    106.0,  // T_cool_max
    1.4,    // P_oil_min
    7.2,    // P_oil_max
    210.0,  // omega_ICE_max_prir
    2250.0, // omega_ICE_max_run
    141.0,  // T_AD_max
    251.0   // T_ballast_max
};

constexpr ModelOutputs testModelOutputs {
    84.0,   // T_cool
    4.1,    // P_oil
    0.0,    // omega_ICE_prir
    1498.0, // omega_ICE_run
    91.0,   // T_AD
    120.0,  // T_ballast
    3190.0, // M_AD
    1498.0  // f_AD
};

// StepOnDemand-тесты управляют временем явно через fake clock.
SimulationConfig
stepOnDemandTestConfig()
{
    return stepOnDemand5Config;
}

// Continuous-тесты используют короткий шаг, чтобы фоновой поток успевал
// выполнить несколько итераций без долгого ожидания в QtTest.
SimulationConfig
continuousTestConfig()
{
    auto config = continuous5Config;
    config.integrationStep = std::chrono::milliseconds{10};
    return config;
}

// Управляемая fake-модель: тесты видят, какие методы вызвал контроллер,
// и сами задают выходы/диагностику модели.
class FakeModelAdapter final : public ModelAdapter {
public:
    bool initializeResult = true;
    bool resetResult = true;
    bool setInputsResult = true;
    bool stepResult = true;

    int initializeCount = 0;
    int resetCount = 0;
    int setInputsCount = 0;
    int stepCount = 0;
    mutable int readOutputsCount = 0;
    mutable int diagnosticsCount = 0;

    ModelInputs lastInputs;
    std::chrono::milliseconds totalStepTime{0};
    std::vector<std::int64_t> stepModelTimes;
    std::vector<std::int64_t> stepDts;
    ModelOutputs outputs;
    emulator::model::diagnostics::ModelDiagnosticsSnapshot diagnosticsSnapshot;

    bool initialize() override
    {
        ++initializeCount;
        return initializeResult;
    }

    bool reset() override
    {
        ++resetCount;
        return resetResult;
    }

    bool setInputs(const ModelInputs& inputs) override
    {
        ++setInputsCount;
        lastInputs = inputs;
        return setInputsResult;
    }

    bool step(std::chrono::milliseconds modelTime,
              std::chrono::milliseconds dt) override
    {
        ++stepCount;
        totalStepTime += dt;
        stepModelTimes.push_back(modelTime.count());
        stepDts.push_back(dt.count());
        return stepResult;
    }

    ModelOutputs readOutputs() const override
    {
        ++readOutputsCount;
        return outputs;
    }

    emulator::model::diagnostics::ModelDiagnosticsSnapshot diagnostics() const override
    {
        ++diagnosticsCount;
        return diagnosticsSnapshot;
    }

    /// @todo Эти методы не используются в тесте
    bool isRunning() const override
    {
        return true;
    }
    bool isEmergency() const override
    {
        return true;
    }
};

// Тестовый exchange проверяет, что bridge читает входной снимок ровно там,
// где должен, и публикует только реально сформированные выходные снимки.
class FakeSnapshotExchange final : public SimulationSnapshotExchange {
public:
    int readCount = 0;
    int publishCount = 0;
    ClientInputSnapshot nextInput;
    std::vector<ModelOutputSnapshot> publishedSnapshots;

    ClientInputSnapshot readClientInputSnapshot() override
    {
        ++readCount;
        return nextInput;
    }

    void publishModelOutputSnapshot(const ModelOutputSnapshot& snapshot) override
    {
        ++publishCount;
        publishedSnapshots.push_back(snapshot);
    }
};

ClientInputSnapshot
startColdRunSnapshot(std::uint64_t revision)
{
    ClientInputSnapshot snapshot;
    snapshot.revision = revision;
    snapshot.command = SimulationCommand::Start;
    snapshot.inputs = ModelInputs{};
    snapshot.inputs->mode = SimulationMode::ColdRun;
    snapshot.inputs->f_AD = 1000.0;
    snapshot.inputs->M_AD_target = 3200.0;
    snapshot.inputs->fan_AD_enabled = false;
    snapshot.inputs->limits = testLimits;
    return snapshot;
}

ClientInputSnapshot
requestSnapshot(std::uint64_t revision, SimulationRequest request)
{
    ClientInputSnapshot snapshot;
    snapshot.revision = revision;
    snapshot.request = request;
    return snapshot;
}

ClientInputSnapshot
commandSnapshot(std::uint64_t revision, SimulationCommand command)
{
    ClientInputSnapshot snapshot;
    snapshot.revision = revision;
    snapshot.command = command;
    return snapshot;
}

bool
hasLimitViolation(const RuntimeDiagnostics& diagnostics, const QString& parameter)
{
    for (const auto& violation : diagnostics.limitViolations) {
        if (violation.parameter == parameter) {
            return true;
        }
    }

    return false;
}

} // namespace

class TestSimulationController : public QObject {
    Q_OBJECT

private slots:
    void startSnapshotInitializesModelAndPublishesStateWithoutOutputs();
    void stepOnDemandDoesNotCalculateBeforeInputsArrive();
    void stepAndReadUsesElapsedTimeBetweenSnapshotsAndIntegrationStep();
    void stepOnDemandReportsLimitViolationsWithoutFaultByDefault();
    void readCurrentStateDoesNotAdvanceModelOrResetStepBoundary();
    void commandsAreHandledOnlyThroughSnapshots();
    void modelFaultPublishesFaultSnapshot();
    void backendBridgeReadsSnapshotAndPublishesControllerOutput();
    void continuousModeCalculatesInBackgroundAndReturnsCurrentState();
    void continuousModeWaitsForInputsBeforeBackgroundCalculation();
    void continuousModeReportsLimitViolationsWithoutStoppingByDefault();
    void continuousModeStopsWhenLimitPolicyRequiresFault();
    void unsupportedRunModeDoesNotDereferenceNullRunner();
};

void
TestSimulationController::startSnapshotInitializesModelAndPublishesStateWithoutOutputs()
{
    FakeModelAdapter model;
    auto now = SimulationController::TimePoint{std::chrono::milliseconds{0}};
    SimulationController controller(model, stepOnDemandTestConfig(), [&now] { return now; });

    // Start переводит симуляцию в Running и инициализирует модель,
    // но в StepOnDemand ещё не выполняет расчётный шаг
    QSignalSpy stateSpy(&controller, &SimulationController::sig_stateChanged);
    const auto snapshot = controller.processSnapshot(startColdRunSnapshot(1));

    QVERIFY(snapshot.has_value());
    QCOMPARE(model.initializeCount, 1);
    QCOMPARE(model.stepCount, 0);
    QCOMPARE(static_cast<int>(controller.state()), static_cast<int>(SimulationState::Running));
    QCOMPARE(snapshot->sourceInputRevision, std::uint64_t{1});
    QCOMPARE(static_cast<int>(snapshot->state), static_cast<int>(SimulationState::Running));
    QVERIFY(!snapshot->outputs.has_value());
    QCOMPARE(snapshot->modelTime.count(), std::int64_t{0});
    QCOMPARE(stateSpy.count(), 1);
}

void
TestSimulationController::stepOnDemandDoesNotCalculateBeforeInputsArrive()
{
    FakeModelAdapter model;
    auto now = SimulationController::TimePoint{std::chrono::milliseconds{0}};
    SimulationController controller(model, stepOnDemandTestConfig(), [&now] { return now; });

    // Start без inputs запускает lifecycle модели, но не должен
    // выполнять расчёт на неявных значениях
    const auto startSnapshot = controller.processSnapshot(
        commandSnapshot(1, SimulationCommand::Start));
    QVERIFY(startSnapshot.has_value());
    QCOMPARE(static_cast<int>(controller.state()), static_cast<int>(SimulationState::Running));

    now += std::chrono::milliseconds{1000};
    const auto noInputsStep = controller.processSnapshot(
        requestSnapshot(2, SimulationRequest::StepAndRead));

    QVERIFY(noInputsStep.has_value());
    QVERIFY(!noInputsStep->outputs.has_value());
    QCOMPARE(model.setInputsCount, 0);
    QCOMPARE(model.stepCount, 0);
    QCOMPARE(controller.modelTime().count(), std::int64_t{0});

    auto firstInputs = requestSnapshot(3, SimulationRequest::StepAndRead);
    firstInputs.inputs = startColdRunSnapshot(3).inputs;
    const auto firstInputsSnapshot = controller.processSnapshot(firstInputs);

    QVERIFY(firstInputsSnapshot.has_value());
    QCOMPARE(model.stepCount, 0);

    now += std::chrono::milliseconds{100};
    const auto calculatedSnapshot = controller.processSnapshot(
        requestSnapshot(4, SimulationRequest::StepAndRead));

    QVERIFY(calculatedSnapshot.has_value());
    QCOMPARE(model.stepCount, 2);
    QCOMPARE(model.lastInputs.f_AD, 1000.0);
    QCOMPARE(model.lastInputs.M_AD_target, 3200.0);
}

void
TestSimulationController::stepAndReadUsesElapsedTimeBetweenSnapshotsAndIntegrationStep()
{
    FakeModelAdapter model;
    auto now = SimulationController::TimePoint{std::chrono::milliseconds{0}};
    SimulationController controller(model, stepOnDemandTestConfig(), [&now] { return now; });

    model.outputs = testModelOutputs;

    // Первый снимок только запускает модель и фиксирует начало интервала
    const auto startSnapshot = controller.processSnapshot(startColdRunSnapshot(1));
    QVERIFY(startSnapshot.has_value());

    // Второй снимок приходит через 1000 мс: контроллер должен разбить
    // этот интервал на 20 внутренних шагов по 50 мс
    now += std::chrono::milliseconds{1000};
    const auto stepRequest = requestSnapshot(2, SimulationRequest::StepAndRead);
    const auto snapshot = controller.processSnapshot(stepRequest);

    QVERIFY(snapshot.has_value());
    QVERIFY(snapshot->outputs.has_value());
    QCOMPARE(model.setInputsCount, 1);
    QCOMPARE(model.stepCount, 20);
    QCOMPARE(model.totalStepTime.count(), std::int64_t{1000});
    QCOMPARE(model.stepModelTimes.front(), std::int64_t{0});
    QCOMPARE(model.stepModelTimes.back(), std::int64_t{950});
    QCOMPARE(model.stepDts.front(), std::int64_t{50});
    QCOMPARE(model.stepDts.back(), std::int64_t{50});
    QCOMPARE(controller.modelTime().count(), std::int64_t{1000});

    QCOMPARE(static_cast<int>(model.lastInputs.mode), static_cast<int>(SimulationMode::ColdRun));
    QCOMPARE(model.lastInputs.f_AD, 1000.0);
    QCOMPARE(model.lastInputs.M_AD_target, 3200.0);
    QCOMPARE(model.lastInputs.limits.T_cool_max, testLimits.T_cool_max);
    QCOMPARE(model.lastInputs.limits.P_oil_min, testLimits.P_oil_min);
    QCOMPARE(model.lastInputs.limits.P_oil_max, testLimits.P_oil_max);
    QCOMPARE(model.lastInputs.limits.omega_ICE_max_prir, testLimits.omega_ICE_max_prir);
    QCOMPARE(model.lastInputs.limits.omega_ICE_max_run, testLimits.omega_ICE_max_run);
    QCOMPARE(model.lastInputs.limits.T_AD_max, testLimits.T_AD_max);
    QCOMPARE(model.lastInputs.limits.T_ballast_max, testLimits.T_ballast_max);
    QCOMPARE(model.lastInputs.fan_AD_enabled, false);

    QCOMPARE(snapshot->sourceInputRevision, std::uint64_t{2});
    QCOMPARE(snapshot->outputs->T_cool, testModelOutputs.T_cool);
    QCOMPARE(snapshot->outputs->P_oil, testModelOutputs.P_oil);
    QCOMPARE(snapshot->outputs->omega_ICE_run, testModelOutputs.omega_ICE_run);
    QCOMPARE(snapshot->outputs->T_AD, testModelOutputs.T_AD);
    QCOMPARE(snapshot->outputs->T_ballast, testModelOutputs.T_ballast);
    QCOMPARE(snapshot->outputs->M_AD, testModelOutputs.M_AD);
    QCOMPARE(snapshot->outputs->f_AD, testModelOutputs.f_AD);
    QCOMPARE(snapshot->modelTime.count(), std::int64_t{1000});
    QVERIFY(snapshot->timestampUtc.isValid());
}

void
TestSimulationController::stepOnDemandReportsLimitViolationsWithoutFaultByDefault()
{
    FakeModelAdapter model;
    auto now = SimulationController::TimePoint{std::chrono::milliseconds{0}};
    SimulationController controller(model, stepOnDemandTestConfig(), [&now] { return now; });
    QSignalSpy faultSpy(&controller, &SimulationController::sig_faultOccurred);

    auto startSnapshot = startColdRunSnapshot(1);
    startSnapshot.inputs->limits.T_cool_max = 70.0;
    startSnapshot.inputs->limits.omega_ICE_max_run = 900.0;
    model.outputs = testModelOutputs;

    const auto startOutput = controller.processSnapshot(startSnapshot);
    QVERIFY(startOutput.has_value());

    now += std::chrono::milliseconds{100};
    const auto stepRequest = requestSnapshot(2, SimulationRequest::StepAndRead);
    const auto snapshot = controller.processSnapshot(stepRequest);

    QVERIFY(snapshot.has_value());
    QVERIFY(snapshot->outputs.has_value());
    QVERIFY(!snapshot->diagnostics.hasFault());
    QVERIFY(snapshot->runtimeDiagnostics.hasLimitViolations());
    QVERIFY(hasLimitViolation(snapshot->runtimeDiagnostics, QStringLiteral("T_cool")));
    QVERIFY(hasLimitViolation(snapshot->runtimeDiagnostics, QStringLiteral("omega_ICE_run")));
    QCOMPARE(static_cast<int>(controller.state()), static_cast<int>(SimulationState::Running));
    QCOMPARE(faultSpy.count(), 0);
}

void
TestSimulationController::readCurrentStateDoesNotAdvanceModelOrResetStepBoundary()
{
    FakeModelAdapter model;
    auto now = SimulationController::TimePoint{std::chrono::milliseconds{0}};
    SimulationController controller(model, stepOnDemandTestConfig(), [&now] { return now; });

    // Сначала выполняем один короткий расчёт на 100 мс
    const auto startSnapshot = controller.processSnapshot(startColdRunSnapshot(1));
    QVERIFY(startSnapshot.has_value());

    now += std::chrono::milliseconds{100};
    const auto firstStep = requestSnapshot(2, SimulationRequest::StepAndRead);
    const auto firstStepSnapshot = controller.processSnapshot(firstStep);
    QVERIFY(firstStepSnapshot.has_value());

    // ReadCurrentState должен только вернуть последнее состояние.
    // Время расчёта и граница следующего StepAndRead не должны измениться.
    now += std::chrono::milliseconds{500};
    const auto readState = requestSnapshot(3, SimulationRequest::ReadCurrentState);
    const auto stateSnapshot = controller.processSnapshot(readState);

    QVERIFY(stateSnapshot.has_value());
    QCOMPARE(model.stepCount, 2);
    QCOMPARE(controller.modelTime().count(), std::int64_t{100});
    QCOMPARE(stateSnapshot->modelTime.count(), std::int64_t{100});

    // Следующий StepAndRead должен досчитать всё время с прошлого расчётного
    // снимка: 500 мс ожидания + 400 мс нового ожидания
    now += std::chrono::milliseconds{400};
    const auto secondStep = requestSnapshot(4, SimulationRequest::StepAndRead);
    const auto secondStepSnapshot = controller.processSnapshot(secondStep);
    QVERIFY(secondStepSnapshot.has_value());

    QCOMPARE(model.totalStepTime.count(), std::int64_t{1000});
    QCOMPARE(controller.modelTime().count(), std::int64_t{1000});
}

void
TestSimulationController::commandsAreHandledOnlyThroughSnapshots()
{
    FakeModelAdapter model;
    auto now = SimulationController::TimePoint{std::chrono::milliseconds{0}};
    SimulationController controller(model, stepOnDemandTestConfig(), [&now] { return now; });

    // У контроллера нет публичных start/stop/reset. Все команды приходят
    // только как часть входного снимка.
    const auto startSnapshot = controller.processSnapshot(startColdRunSnapshot(1));
    QVERIFY(startSnapshot.has_value());
    QCOMPARE(static_cast<int>(controller.state()), static_cast<int>(SimulationState::Running));

    const auto stopSnapshot = commandSnapshot(2, SimulationCommand::Stop);
    const auto stoppedSnapshot = controller.processSnapshot(stopSnapshot);
    QVERIFY(stoppedSnapshot.has_value());
    QCOMPARE(static_cast<int>(controller.state()), static_cast<int>(SimulationState::Stopped));

    const auto resetSnapshot = commandSnapshot(3, SimulationCommand::Reset);
    const auto resetResultSnapshot = controller.processSnapshot(resetSnapshot);
    QVERIFY(resetResultSnapshot.has_value());
    QCOMPARE(model.resetCount, 1);
    QCOMPARE(static_cast<int>(controller.state()), static_cast<int>(SimulationState::Stopped));
    QCOMPARE(controller.modelTime().count(), std::int64_t{0});

    const auto emergencyStopSnapshot = commandSnapshot(4, SimulationCommand::EmergencyStop);
    const auto faultSnapshot = controller.processSnapshot(emergencyStopSnapshot);
    QVERIFY(faultSnapshot.has_value());
    QCOMPARE(static_cast<int>(controller.state()), static_cast<int>(SimulationState::Fault));
    QVERIFY(faultSnapshot->diagnostics.hasFault());
    QCOMPARE(faultSnapshot->diagnostics.faultCode, SimulationFaultCode::EmergencyStop);
}

void
TestSimulationController::modelFaultPublishesFaultSnapshot()
{
    FakeModelAdapter model;
    auto now = SimulationController::TimePoint{std::chrono::milliseconds{0}};
    SimulationController controller(model, stepOnDemandTestConfig(), [&now] { return now; });

    QSignalSpy faultSpy(&controller, &SimulationController::sig_faultOccurred);
    const auto startSnapshot = controller.processSnapshot(startColdRunSnapshot(1));
    QVERIFY(startSnapshot.has_value());

    // Диагностика модели имеет приоритет: если adapter сообщает fault,
    // контроллер переводит симуляцию в Fault и отдаёт fault-снимок наружу
    model.diagnosticsSnapshot.faultCode = testModelFaultCode;
    model.diagnosticsSnapshot.message = QStringLiteral("modelFaultPublishesFaultSnapshot");

    now += std::chrono::milliseconds{100};
    const auto stepRequest = requestSnapshot(2, SimulationRequest::StepAndRead);
    const auto faultSnapshot = controller.processSnapshot(stepRequest);

    QVERIFY(faultSnapshot.has_value());
    QCOMPARE(static_cast<int>(controller.state()), static_cast<int>(SimulationState::Fault));
    QVERIFY(faultSnapshot->diagnostics.hasFault());
    QCOMPARE(faultSnapshot->diagnostics.faultCode, SimulationFaultCode::ModelAdapterFault);
    QCOMPARE(faultSnapshot->diagnostics.message,
             QStringLiteral("modelFaultPublishesFaultSnapshot"));
    QCOMPARE(faultSpy.count(), 1);

    const auto arguments = faultSpy.takeFirst();
    const auto diagnostics = arguments.at(0).value<DiagnosticsSnapshot>();
    QVERIFY(diagnostics.hasFault());
    QCOMPARE(diagnostics.faultCode, SimulationFaultCode::ModelAdapterFault);
}

void
TestSimulationController::backendBridgeReadsSnapshotAndPublishesControllerOutput()
{
    FakeSnapshotExchange exchange;
    FakeModelAdapter model;
    auto now = SimulationController::TimePoint{std::chrono::milliseconds{0}};
    SimulationController controller(model, stepOnDemandTestConfig(), [&now] { return now; });
    SimulationBackendBridge bridge(exchange, controller);

    QSignalSpy bridgePublishSpy(&bridge, &SimulationBackendBridge::sig_outputPublished);
    QSignalSpy noOutputSpy(&bridge, &SimulationBackendBridge::sig_noOutputProduced);

    // Bridge читает один входной снимок из exchange и публикует то,
    // что вернул контроллер. Start даёт state-снимок без outputs.
    exchange.nextInput = startColdRunSnapshot(1);
    bridge.slot_processRequest();

    QCOMPARE(exchange.readCount, 1);
    QCOMPARE(exchange.publishCount, 1);
    QCOMPARE(bridgePublishSpy.count(), 1);
    QCOMPARE(noOutputSpy.count(), 0);
    QVERIFY(!exchange.publishedSnapshots.back().outputs.has_value());

    model.outputs = testModelOutputs;
    now += std::chrono::milliseconds{100};
    exchange.nextInput = requestSnapshot(2, SimulationRequest::StepAndRead);
    bridge.slot_processRequest();

    QCOMPARE(exchange.readCount, 2);
    QCOMPARE(exchange.publishCount, 2);
    QCOMPARE(bridgePublishSpy.count(), 2);
    QCOMPARE(model.stepCount, 2);
    QCOMPARE(exchange.publishedSnapshots.back().sourceInputRevision, std::uint64_t{2});
    QVERIFY(exchange.publishedSnapshots.back().outputs.has_value());
    QCOMPARE(exchange.publishedSnapshots.back().outputs->T_cool, testModelOutputs.T_cool);

    // Пустой снимок не требует публикации выходов
    exchange.nextInput = ClientInputSnapshot{};
    exchange.nextInput.revision = 3;
    bridge.slot_processRequest();

    QCOMPARE(exchange.readCount, 3);
    QCOMPARE(exchange.publishCount, 2);
    QCOMPARE(noOutputSpy.count(), 1);
}

void
TestSimulationController::continuousModeCalculatesInBackgroundAndReturnsCurrentState()
{
    FakeModelAdapter model;
    SimulationController controller(model, continuousTestConfig());

    auto startSnapshot = startColdRunSnapshot(1);
    startSnapshot.inputs->limits.P_oil_min = 0.0;
    startSnapshot.inputs->limits.omega_ICE_max_run = 3000.0;
    model.outputs = testModelOutputs;

    // В Continuous режиме запускается фоновый расчётный поток
    const auto startOutput = controller.processSnapshot(startSnapshot);
    QVERIFY(startOutput.has_value());
    QVERIFY(!startOutput->outputs.has_value());
    QCOMPARE(static_cast<int>(controller.state()), static_cast<int>(SimulationState::Running));

    QTRY_VERIFY_WITH_TIMEOUT(controller.modelTime().count() >= std::int64_t{20}, 1000);

    // Чтение состояния не делает отдельный step, а возвращает уже накопленные
    // данные фонового расчёта
    const auto readState = requestSnapshot(2, SimulationRequest::ReadCurrentState);
    const auto currentState = controller.processSnapshot(readState);

    QVERIFY(currentState.has_value());
    QVERIFY(currentState->outputs.has_value());
    QCOMPARE(currentState->outputs->P_oil, testModelOutputs.P_oil);
    QCOMPARE(static_cast<int>(currentState->state), static_cast<int>(SimulationState::Running));
    QVERIFY(currentState->modelTime >= std::chrono::milliseconds{20});

    const auto stopSnapshot = commandSnapshot(3, SimulationCommand::Stop);
    const auto stopped = controller.processSnapshot(stopSnapshot);
    QVERIFY(stopped.has_value());
    QCOMPARE(static_cast<int>(controller.state()), static_cast<int>(SimulationState::Stopped));
}

void
TestSimulationController::continuousModeWaitsForInputsBeforeBackgroundCalculation()
{
    FakeModelAdapter model;
    SimulationController controller(model, continuousTestConfig());

    // Continuous можно запустить без inputs, но worker должен ждать реальный
    // входной снимок.
    const auto startOutput = controller.processSnapshot(
        commandSnapshot(1, SimulationCommand::Start));
    QVERIFY(startOutput.has_value());
    QCOMPARE(static_cast<int>(controller.state()), static_cast<int>(SimulationState::Running));

    QTest::qWait(50);
    QCOMPARE(model.setInputsCount, 0);
    QCOMPARE(model.stepCount, 0);
    QCOMPARE(controller.modelTime().count(), std::int64_t{0});

    auto inputSnapshot = requestSnapshot(2, SimulationRequest::None);
    inputSnapshot.inputs = startColdRunSnapshot(2).inputs;
    model.outputs = testModelOutputs;

    const auto noOutput = controller.processSnapshot(inputSnapshot);
    QVERIFY(!noOutput.has_value());

    QTRY_VERIFY_WITH_TIMEOUT(model.stepCount > 0, 1000);
    QCOMPARE(model.lastInputs.f_AD, 1000.0);
    QCOMPARE(model.lastInputs.M_AD_target, 3200.0);

    const auto stopSnapshot = commandSnapshot(3, SimulationCommand::Stop);
    const auto stopped = controller.processSnapshot(stopSnapshot);
    QVERIFY(stopped.has_value());
}

void
TestSimulationController::continuousModeReportsLimitViolationsWithoutStoppingByDefault()
{
    FakeModelAdapter model;
    SimulationController controller(model, continuousTestConfig());
    QSignalSpy faultSpy(&controller, &SimulationController::sig_faultOccurred);

    auto startSnapshot = startColdRunSnapshot(1);
    startSnapshot.inputs->limits.T_cool_max = 70.0;
    startSnapshot.inputs->limits.P_oil_min = 0.0;
    startSnapshot.inputs->limits.omega_ICE_max_run = 900.0;
    startSnapshot.inputs->limits.T_ballast_max = 80.0;
    model.outputs = testModelOutputs;

    // По умолчанию превышения лимитов публикуются как runtime-диагностика,
    // но не переводят simulation в Fault.
    const auto startOutput = controller.processSnapshot(startSnapshot);
    QVERIFY(startOutput.has_value());

    QTRY_VERIFY_WITH_TIMEOUT(controller.modelTime().count() >= std::int64_t{10}, 1000);
    QCOMPARE(static_cast<int>(controller.state()), static_cast<int>(SimulationState::Running));
    QCOMPARE(faultSpy.count(), 0);

    const auto readState = requestSnapshot(2, SimulationRequest::ReadCurrentState);
    const auto state = controller.processSnapshot(readState);
    QVERIFY(state.has_value());
    QVERIFY(!state->diagnostics.hasFault());
    QVERIFY(state->runtimeDiagnostics.hasLimitViolations());
    QVERIFY(hasLimitViolation(state->runtimeDiagnostics, QStringLiteral("T_cool")));
    QVERIFY(hasLimitViolation(state->runtimeDiagnostics, QStringLiteral("omega_ICE_run")));
    QVERIFY(hasLimitViolation(state->runtimeDiagnostics, QStringLiteral("T_ballast")));

    const auto stopSnapshot = commandSnapshot(3, SimulationCommand::Stop);
    const auto stopped = controller.processSnapshot(stopSnapshot);
    QVERIFY(stopped.has_value());
}

void
TestSimulationController::continuousModeStopsWhenLimitPolicyRequiresFault()
{
    FakeModelAdapter model;
    auto config = continuousTestConfig();
    config.limitViolationAction = LimitViolationAction::StopSimulation;
    SimulationController controller(model, config);
    QSignalSpy faultSpy(&controller, &SimulationController::sig_faultOccurred);

    auto startSnapshot = startColdRunSnapshot(1);
    startSnapshot.inputs->limits.T_cool_max = 70.0;
    startSnapshot.inputs->limits.P_oil_min = 0.0;
    startSnapshot.inputs->limits.omega_ICE_max_run = 900.0;
    startSnapshot.inputs->limits.T_ballast_max = 80.0;
    model.outputs = testModelOutputs;

    const auto startOutput = controller.processSnapshot(startSnapshot);
    QVERIFY(startOutput.has_value());

    QTRY_COMPARE_WITH_TIMEOUT(faultSpy.count(), 1, 1000);
    QCOMPARE(static_cast<int>(controller.state()), static_cast<int>(SimulationState::Fault));

    const auto arguments = faultSpy.takeFirst();
    const auto diagnostics = arguments.at(0).value<DiagnosticsSnapshot>();
    QVERIFY(diagnostics.hasFault());
    QCOMPARE(diagnostics.faultCode, SimulationFaultCode::SimulationLimitsExceeded);
    QVERIFY(diagnostics.message.contains(QStringLiteral("T_cool")));
    QVERIFY(diagnostics.message.contains(QStringLiteral("omega_ICE_run")));
    QVERIFY(diagnostics.message.contains(QStringLiteral("T_ballast")));

    const auto readState = requestSnapshot(2, SimulationRequest::ReadCurrentState);
    const auto faultState = controller.processSnapshot(readState);
    QVERIFY(faultState.has_value());
    QVERIFY(faultState->diagnostics.hasFault());
    QVERIFY(faultState->runtimeDiagnostics.hasLimitViolations());
}

void
TestSimulationController::unsupportedRunModeDoesNotDereferenceNullRunner()
{
    FakeModelAdapter model;
    SimulationConfig config;
    config.runMode = static_cast<SimulationRunMode>(255);

    SimulationController controller(model, config);
    QSignalSpy faultSpy(&controller, &SimulationController::sig_faultOccurred);

    // Если фабрика стратегий вернула nullptr, публичный API контроллера
    // должен остаться безопасным и не разыменовывать пустой runner.
    const auto output = controller.processSnapshot(startColdRunSnapshot(1));

    QVERIFY(!output.has_value());
    QCOMPARE(static_cast<int>(controller.state()), static_cast<int>(SimulationState::Fault));
    QCOMPARE(controller.isRunning(), false);
    QCOMPARE(controller.modelTime().count(), std::int64_t{0});
    QVERIFY(!controller.lastOutputSnapshot().has_value());
    QCOMPARE(model.initializeCount, 0);
    QCOMPARE(model.stepCount, 0);
    QCOMPARE(faultSpy.count(), 1);

    const auto arguments = faultSpy.takeFirst();
    const auto diagnostics = arguments.at(0).value<DiagnosticsSnapshot>();
    QVERIFY(diagnostics.hasFault());
    QCOMPARE(diagnostics.faultCode, SimulationFaultCode::UnsupportedRunMode);
    QVERIFY(diagnostics.message.contains(QStringLiteral("Unsupported simulation run mode")));
}

QTEST_MAIN(TestSimulationController)

#include "test_api.moc"

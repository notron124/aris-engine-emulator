#include <QtTest/QSignalSpy>
#include <QtTest/QTest>

#include <QModbusDataUnit>
#include <QModbusReply>
#include <QModbusTcpClient>
#include <QDateTime>
#include <QTimeZone>

#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <memory>
#include <vector>

#include "ModbusServer.hpp"
#include "ModelAdapter.hpp"
#include "RegisterBank.hpp"
#include "simulation/controller/SimulationController.hpp"
#include "simulation/exchange/SimulationBackendBridge.hpp"

namespace {

using emulator::modbus::ModbusServer;
using emulator::registerbank::RegisterBank;
using emulator::registerbank::SimulationSnapshotExchangeImpl;
using emulator::simulation::SimulationBackendBridge;
using emulator::simulation::SimulationCommand;
using emulator::simulation::SimulationController;
using emulator::simulation::SimulationMode;
using emulator::simulation::SimulationRequest;
using emulator::simulation::SimulationState;
using emulator::simulation::LimitViolationAction;
using emulator::simulation::diagnostics::SimulationFaultCode;

constexpr auto testHost = "127.0.0.1";
constexpr quint16 testPort = 15020;
constexpr int modbusServerAddress = 1;
constexpr qsizetype doubleRegisterCount =
    static_cast<qsizetype>(sizeof(double) / sizeof(quint16));
constexpr qsizetype uint64RegisterCount =
    static_cast<qsizetype>(sizeof(std::uint64_t) / sizeof(quint16));

struct PublishedRegistersSnapshot {
    std::uint64_t revision = 0;
    std::uint64_t sourceInputRevision = 0;
    std::uint64_t modelTime = 0;
    std::uint64_t timestampUtcMs = 0;
    SimulationState state = SimulationState::Stopped;
    SimulationFaultCode faultCode = SimulationFaultCode::None;
    bool hasFault = false;
    bool hasLimitViolations = false;
    emulator::model::ModelOutputs outputs;
};

class ScenarioModelAdapter final : public emulator::model::ModelAdapter {
public:
    bool initialize() override
    {
        initialized_ = true;
        running_ = true;
        fault_ = false;
        modelTime_ = std::chrono::milliseconds{0};
        outputs_ = outputsForMode(inputs_);
        return true;
    }

    bool reset() override
    {
        initialized_ = false;
        running_ = false;
        fault_ = false;
        inputs_ = emulator::model::ModelInputs{};
        outputs_ = emulator::model::ModelOutputs{};
        modelTime_ = std::chrono::milliseconds{0};
        modeHistory.clear();
        return true;
    }

    bool setInputs(const emulator::model::ModelInputs& inputs) override
    {
        inputs_ = inputs;
        modeHistory.push_back(inputs.mode);
        return true;
    }

    bool step(std::chrono::milliseconds modelTime,
              std::chrono::milliseconds dt) override
    {
        if (!initialized_ || !running_ || fault_) {
            return false;
        }

        modelTime_ = modelTime + dt;
        outputs_ = outputsForMode(inputs_);
        return true;
    }

    [[nodiscard]] emulator::model::ModelOutputs readOutputs() const override
    {
        return outputs_;
    }

    [[nodiscard]] emulator::model::diagnostics::ModelDiagnosticsSnapshot diagnostics() const override
    {
        emulator::model::diagnostics::ModelDiagnosticsSnapshot diagnostics;
        if (fault_) {
            diagnostics.faultCode =
                emulator::model::diagnostics::ModelFaultCode::ModelFault;
            diagnostics.message = QStringLiteral("Scenario model fault");
        }
        return diagnostics;
    }

    [[nodiscard]] bool isRunning() const override
    {
        return running_ && !fault_;
    }

    [[nodiscard]] bool isEmergency() const override
    {
        return fault_;
    }

    std::vector<SimulationMode> modeHistory;

private:
    static emulator::model::ModelOutputs outputsForMode(
        const emulator::model::ModelInputs& inputs)
    {
        emulator::model::ModelOutputs outputs;

        switch (inputs.mode) {
        case SimulationMode::ColdRun:
            outputs.T_cool = 35.0;
            outputs.P_oil = 2.1;
            outputs.omega_ICE_prir = 110.0;
            outputs.omega_ICE_run = 110.0;
            outputs.T_AD = 31.0;
            outputs.T_ballast = 30.0;
            outputs.M_AD = 0.0;
            outputs.f_AD = inputs.f_AD;
            break;

        case SimulationMode::StartWarmup:
            outputs.T_cool = 52.0;
            outputs.P_oil = 2.6;
            outputs.omega_ICE_prir = 240.0;
            outputs.omega_ICE_run = 240.0;
            outputs.T_AD = 38.0;
            outputs.T_ballast = 34.0;
            outputs.M_AD = 10.0;
            outputs.f_AD = inputs.f_AD;
            break;

        case SimulationMode::HotNoLoad:
            outputs.T_cool = 74.0;
            outputs.P_oil = 3.0;
            outputs.omega_ICE_prir = 500.0;
            outputs.omega_ICE_run = 500.0;
            outputs.T_AD = 48.0;
            outputs.T_ballast = 42.0;
            outputs.M_AD = 0.0;
            outputs.f_AD = inputs.f_AD;
            break;

        case SimulationMode::HotLoad:
            outputs.T_cool = 88.0;
            outputs.P_oil = 3.4;
            outputs.omega_ICE_prir = 850.0;
            outputs.omega_ICE_run = 850.0;
            outputs.T_AD = 72.0;
            outputs.T_ballast = 95.0;
            outputs.M_AD = inputs.M_AD_target;
            outputs.f_AD = inputs.f_AD;
            break;
        }

        outputs.ice_rpm = outputs.omega_ICE_run;
        outputs.t_cool_c = outputs.T_cool;
        outputs.t_ad_c = outputs.T_AD;
        outputs.t_ballast_c = outputs.T_ballast;
        outputs.p_oil_bar = outputs.P_oil;
        outputs.m_ad_nm = outputs.M_AD;
        return outputs;
    }

    bool initialized_ = false;
    bool running_ = false;
    bool fault_ = false;
    std::chrono::milliseconds modelTime_{0};
    emulator::model::ModelInputs inputs_;
    emulator::model::ModelOutputs outputs_;
};

void putDouble(QList<quint16>& registers, qsizetype offset, double value)
{
    std::memcpy(registers.data() + offset, &value, sizeof(value));
}

void putUInt64(QList<quint16>& registers, qsizetype offset, std::uint64_t value)
{
    std::memcpy(registers.data() + offset, &value, sizeof(value));
}

std::uint64_t readUInt64(const QModbusDataUnit& unit)
{
    std::uint64_t value = 0;
    const auto values = unit.values();
    std::memcpy(&value, values.constData(), sizeof(value));
    return value;
}

double readDouble(const QList<quint16>& registers)
{
    double value = 0.0;
    std::memcpy(&value, registers.constData(), sizeof(value));
    return value;
}

double readDoubleAt(const QList<quint16>& registers, qsizetype offset)
{
    double value = 0.0;
    std::memcpy(&value, registers.constData() + offset, sizeof(value));
    return value;
}

std::uint64_t readUInt64(const QList<quint16>& registers)
{
    std::uint64_t value = 0;
    std::memcpy(&value, registers.constData(), sizeof(value));
    return value;
}

std::uint64_t readUInt64At(const QList<quint16>& registers, qsizetype offset)
{
    std::uint64_t value = 0;
    std::memcpy(&value, registers.constData() + offset, sizeof(value));
    return value;
}

double readInputDouble(const RegisterBank& registerBank, quint16 offset)
{
    return readDouble(registerBank.readInputRegisters(
        offset,
        static_cast<quint16>(doubleRegisterCount)));
}

std::uint64_t readInputUInt64(const RegisterBank& registerBank, quint16 offset)
{
    return readUInt64(registerBank.readInputRegisters(
        offset,
        static_cast<quint16>(uint64RegisterCount)));
}

emulator::model::ModelOutputs makeModelOutputs()
{
    emulator::model::ModelOutputs outputs;
    outputs.T_cool = 78.5;
    outputs.P_oil = 3.25;
    outputs.omega_ICE_prir = 120.0;
    outputs.omega_ICE_run = 950.0;
    outputs.T_AD = 66.0;
    outputs.T_ballast = 71.5;
    outputs.M_AD = 185.0;
    outputs.f_AD = 49.5;
    return outputs;
}

emulator::exchange::ModelOutputSnapshot makeModelOutputSnapshot(
    std::uint64_t revision,
    std::uint64_t sourceInputRevision)
{
    emulator::exchange::ModelOutputSnapshot snapshot;
    snapshot.revision = revision;
    snapshot.sourceInputRevision = sourceInputRevision;
    snapshot.timestampUtc = QDateTime::fromMSecsSinceEpoch(
        1712345678000,
        QTimeZone(QByteArrayLiteral("UTC")));
    snapshot.state = SimulationState::Running;
    snapshot.outputs = makeModelOutputs();
    snapshot.modelTime = std::chrono::milliseconds{1250};
    return snapshot;
}

QList<quint16> makeStartColdRunRequest(
    std::uint64_t revision,
    SimulationCommand command = SimulationCommand::Start,
    SimulationRequest request = SimulationRequest::ReadCurrentState,
    SimulationMode mode = SimulationMode::ColdRun)
{
    QList<quint16> registers(RegisterBank::simulationMode + 1);
    registers.fill(0);

    // Лимиты пишем явно: RegisterBank превращает holding registers в
    // ClientInputSnapshot, и нулевые лимиты сразу сделали бы модель аварийной.
    putDouble(registers, RegisterBank::T_cool_max, 105.0);
    putDouble(registers, RegisterBank::P_oil_min, 1.5);
    putDouble(registers, RegisterBank::P_oil_max, 7.0);
    putDouble(registers, RegisterBank::omega_ICE_max_prir, 200.0);
    putDouble(registers, RegisterBank::omega_ICE_max_run, 2000.0);
    putDouble(registers, RegisterBank::rpm_max_lapping, 200.0);
    putDouble(registers, RegisterBank::rpm_max_run, 2000.0);
    putDouble(registers, RegisterBank::target_brake_torque_nm, 100.0);
    putDouble(registers, RegisterBank::throttle_position, 0.25);
    putDouble(registers, RegisterBank::T_AD_max, 140.0);
    putDouble(registers, RegisterBank::T_ballast_max, 250.0);
    putDouble(registers, RegisterBank::f_AD_Input, 50.0);
    putDouble(registers, RegisterBank::M_AD_target, 100.0);
    putUInt64(registers, RegisterBank::revision_h, revision);

    registers[RegisterBank::simulationCommand] =
        static_cast<quint16>(command);
    registers[RegisterBank::simulationRequest] =
        static_cast<quint16>(request);
    registers[RegisterBank::simulationMode] =
        static_cast<quint16>(mode);

    return registers;
}

QList<quint16> makeLifecycleRequest(
    std::uint64_t revision,
    SimulationCommand command,
    SimulationRequest request,
    SimulationMode mode,
    bool violateLimits = false)
{
    auto registers = makeStartColdRunRequest(revision, command, request, mode);

    putDouble(registers, RegisterBank::P_oil_min, -1.0);
    putDouble(registers, RegisterBank::P_oil_max, 20.0);
    putDouble(registers, RegisterBank::T_cool_max, 150.0);
    putDouble(registers, RegisterBank::T_AD_max, 150.0);
    putDouble(registers, RegisterBank::T_ballast_max, 200.0);
    putDouble(registers, RegisterBank::rpm_max_lapping, 5000.0);
    putDouble(registers, RegisterBank::rpm_max_run, 5000.0);

    const double runtimeOmegaLimit = violateLimits ? 50.0 : 5000.0;
    putDouble(registers, RegisterBank::omega_ICE_max_prir, runtimeOmegaLimit);
    putDouble(registers, RegisterBank::omega_ICE_max_run, runtimeOmegaLimit);

    switch (mode) {
    case SimulationMode::ColdRun:
        putDouble(registers, RegisterBank::f_AD_Input, 10.0);
        putDouble(registers, RegisterBank::M_AD_target, 0.0);
        break;

    case SimulationMode::StartWarmup:
        putDouble(registers, RegisterBank::f_AD_Input, 20.0);
        putDouble(registers, RegisterBank::M_AD_target, 10.0);
        break;

    case SimulationMode::HotNoLoad:
        putDouble(registers, RegisterBank::f_AD_Input, 35.0);
        putDouble(registers, RegisterBank::M_AD_target, 0.0);
        break;

    case SimulationMode::HotLoad:
        putDouble(registers, RegisterBank::f_AD_Input, 50.0);
        putDouble(registers, RegisterBank::M_AD_target, 120.0);
        break;
    }

    return registers;
}

} // namespace

class TestIntegration : public QObject {
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void registerBankBuildsClientInputSnapshotFromRegisters();
    void registerBankPublishesModelOutputSnapshotToRegisters();
    void snapshotExchangePassesSnapshotsThroughRegisterBank();
    void modelAdapterAcceptsModelInputsAndReturnsModelOutputs();
    void mainCompositionPublishesSimulationSnapshotAfterModbusWrite();
    void clientRunsFullModeSwitchingLifecycleWithoutLimitViolation();
    void clientRunsModeSwitchingLifecycleAndPublishesLimitViolation();

private:
    void waitAndCheck(QModbusReply* reply);
    void waitForPublishedCount(QSignalSpy& spy, int expectedCount);
    void writeClientInputSnapshot(
        std::uint64_t revision,
        SimulationCommand command,
        SimulationRequest request,
        SimulationMode mode,
        bool violateLimits = false);
    QModbusDataUnit readModbusDataUnit(
        QModbusDataUnit::RegisterType type,
        quint16 startAddress,
        quint16 valueCount);
    PublishedRegistersSnapshot readPublishedRegistersSnapshot();
    void switchModeAndReadSnapshot(
        QSignalSpy& outputPublishedSpy,
        int& expectedPublications,
        std::uint64_t& revision,
        SimulationMode mode,
        PublishedRegistersSnapshot& snapshot,
        bool violateLimits = false);

    std::unique_ptr<RegisterBank> registerBank_;
    std::unique_ptr<ModbusServer> server_;
    std::unique_ptr<emulator::model::ModelAdapter> model_;
    std::unique_ptr<SimulationController> controller_;
    std::unique_ptr<SimulationSnapshotExchangeImpl> exchange_;
    std::unique_ptr<SimulationBackendBridge> bridge_;
    std::unique_ptr<QModbusTcpClient> client_;
};

void TestIntegration::init()
{
    // Повторяем composition root из main.cpp, но оставляем объекты под
    // управлением теста, чтобы можно было дождаться сигналов и корректно
    // остановить сервер в cleanup().
    auto config = emulator::simulation::continuous5Config;
    const auto currentTest = QByteArray{QTest::currentTestFunction()};
    const bool useScenarioModel =
        currentTest == "clientRunsFullModeSwitchingLifecycleWithoutLimitViolation"
        || currentTest == "clientRunsModeSwitchingLifecycleAndPublishesLimitViolation";

    if (currentTest == "clientRunsModeSwitchingLifecycleAndPublishesLimitViolation") {
        config.limitViolationAction = LimitViolationAction::StopSimulation;
    }

    registerBank_ = std::make_unique<RegisterBank>();
    server_ = std::make_unique<ModbusServer>(registerBank_.get());
    if (useScenarioModel) {
        model_ = std::make_unique<ScenarioModelAdapter>();
    } else {
        model_ = std::make_unique<emulator::model::ModelBase>();
    }
    controller_ = std::make_unique<SimulationController>(
        *model_,
        config);
    exchange_ = std::make_unique<SimulationSnapshotExchangeImpl>(registerBank_.get());
    bridge_ = std::make_unique<SimulationBackendBridge>(*exchange_, *controller_);

    connect(registerBank_.get(),
            &RegisterBank::sig_inputSnapshotUpdated,
            bridge_.get(),
            &SimulationBackendBridge::slot_processRequest);

    QVERIFY(server_->start(QString::fromLatin1(testHost), testPort));

    client_ = std::make_unique<QModbusTcpClient>();
    client_->setConnectionParameter(QModbusDevice::NetworkAddressParameter,
                                    QString::fromLatin1(testHost));
    client_->setConnectionParameter(QModbusDevice::NetworkPortParameter, testPort);

    QVERIFY(client_->connectDevice());

    QSignalSpy connectedSpy(client_.get(), &QModbusClient::stateChanged);
    if (client_->state() != QModbusDevice::ConnectedState) {
        QVERIFY(connectedSpy.wait(1000));
    }
    QCOMPARE(client_->state(), QModbusDevice::ConnectedState);
}

void TestIntegration::cleanup()
{
    if (client_) {
        client_->disconnectDevice();
    }

    bridge_.reset();
    exchange_.reset();
    controller_.reset();
    model_.reset();
    server_.reset();
    registerBank_.reset();
    client_.reset();
}

void TestIntegration::registerBankBuildsClientInputSnapshotFromRegisters()
{
    RegisterBank registerBank;
    QVERIFY(registerBank.writeCoils(0, QList<bool>{true, false, true}));

    const auto registers = makeStartColdRunRequest(
        42,
        SimulationCommand::Start,
        SimulationRequest::StepAndRead,
        SimulationMode::HotLoad);
    QVERIFY(registerBank.writeHoldingRegisters(0, registers));

    // Проверяем границу RegisterBank -> ClientInputSnapshot: сырые регистры и
    // coils должны стать нормализованным DTO для SimulationController.
    const auto snapshot = registerBank.RegBankSendInfo();
    QCOMPARE(snapshot.revision, std::uint64_t{42});
    QCOMPARE(static_cast<int>(snapshot.command),
             static_cast<int>(SimulationCommand::Start));
    QCOMPARE(static_cast<int>(snapshot.request),
             static_cast<int>(SimulationRequest::StepAndRead));
    QVERIFY(snapshot.inputs.has_value());

    const auto& inputs = *snapshot.inputs;
    QCOMPARE(static_cast<int>(inputs.mode),
             static_cast<int>(SimulationMode::HotLoad));
    QCOMPARE(inputs.f_AD, 50.0);
    QCOMPARE(inputs.M_AD_target, 100.0);
    QCOMPARE(inputs.stator_frequency_hz, inputs.f_AD);
    QCOMPARE(inputs.target_torque_nm, inputs.M_AD_target);
    QCOMPARE(inputs.target_brake_torque_nm, inputs.M_AD_target);
    QCOMPARE(inputs.throttle_position, 0.25);
    QCOMPARE(inputs.fan_ICE_enabled, true);
    QCOMPARE(inputs.fan_AD_enabled, false);
    QCOMPARE(inputs.fan_ballast_enabled, true);
    QCOMPARE(inputs.limits.T_cool_max, 105.0);
    QCOMPARE(inputs.limits.P_oil_min, 1.5);
    QCOMPARE(inputs.limits.P_oil_max, 7.0);
    QCOMPARE(inputs.limits.omega_ICE_max_prir, 200.0);
    QCOMPARE(inputs.limits.omega_ICE_max_run, 2000.0);
    QCOMPARE(inputs.limits.rpm_max_lapping, 200.0);
    QCOMPARE(inputs.limits.rpm_max_run, 2000.0);
    QCOMPARE(inputs.limits.T_AD_max, 140.0);
    QCOMPARE(inputs.limits.T_ballast_max, 250.0);
}

void TestIntegration::registerBankPublishesModelOutputSnapshotToRegisters()
{
    RegisterBank registerBank;
    auto snapshot = makeModelOutputSnapshot(7, 42);
    snapshot.diagnostics.faultCode = SimulationFaultCode::EmergencyStop;
    snapshot.diagnostics.message = QStringLiteral("test emergency stop");
    snapshot.runtimeDiagnostics.limitViolations.push_back({
        QStringLiteral("T_cool"),
        120.0,
        QStringLiteral(">"),
        105.0
    });

    // Проверяем обратную границу ModelOutputSnapshot -> регистры: RegisterBank
    // должен разложить DTO по Input Registers и Discrete Inputs.
    QVERIFY(registerBank.RegBankTakeInfo(snapshot));
    QCOMPARE(readInputUInt64(registerBank, RegisterBank::revision),
             std::uint64_t{7});
    QCOMPARE(readInputUInt64(registerBank, RegisterBank::sourceInputRevision),
             std::uint64_t{42});
    QCOMPARE(readInputUInt64(registerBank, RegisterBank::modelTime),
             std::uint64_t{1250});
    QCOMPARE(readInputUInt64(registerBank, RegisterBank::timestamp_ir),
             static_cast<std::uint64_t>(
                 snapshot.timestampUtc.toMSecsSinceEpoch()));

    const auto& outputs = *snapshot.outputs;
    QCOMPARE(readInputDouble(registerBank, RegisterBank::T_cool),
             outputs.T_cool);
    QCOMPARE(readInputDouble(registerBank, RegisterBank::P_oil),
             outputs.P_oil);
    QCOMPARE(readInputDouble(registerBank, RegisterBank::omega_ICE_prir),
             outputs.omega_ICE_prir);
    QCOMPARE(readInputDouble(registerBank, RegisterBank::omega_ICE_run),
             outputs.omega_ICE_run);
    QCOMPARE(readInputDouble(registerBank, RegisterBank::T_AD),
             outputs.T_AD);
    QCOMPARE(readInputDouble(registerBank, RegisterBank::T_ballast),
             outputs.T_ballast);
    QCOMPARE(readInputDouble(registerBank, RegisterBank::M_AD),
             outputs.M_AD);
    QCOMPARE(readInputDouble(registerBank, RegisterBank::f_AD),
             outputs.f_AD);

    QCOMPARE(registerBank.readInputRegisters(RegisterBank::state_ir, 1).at(0),
             static_cast<quint16>(SimulationState::Running));
    QCOMPARE(registerBank.readInputRegisters(RegisterBank::faultCode, 1).at(0),
             static_cast<quint16>(SimulationFaultCode::EmergencyStop));

    const auto flags = registerBank.readDiscreteInputs(RegisterBank::hasFault, 2);
    QCOMPARE(flags.at(0), true);
    QCOMPARE(flags.at(1), true);
}

void TestIntegration::snapshotExchangePassesSnapshotsThroughRegisterBank()
{
    RegisterBank registerBank;
    SimulationSnapshotExchangeImpl exchange(&registerBank);

    QVERIFY(registerBank.writeCoils(0, QList<bool>{true, true, false}));
    QVERIFY(registerBank.writeHoldingRegisters(
        0,
        makeStartColdRunRequest(
            9,
            SimulationCommand::None,
            SimulationRequest::ReadCurrentState,
            SimulationMode::StartWarmup)));

    // SimulationSnapshotExchangeImpl не добавляет бизнес-логику: он только
    // читает входной снимок из RegisterBank и публикует выходной обратно.
    const auto inputSnapshot = exchange.readClientInputSnapshot();
    QCOMPARE(inputSnapshot.revision, std::uint64_t{9});
    QCOMPARE(static_cast<int>(inputSnapshot.command),
             static_cast<int>(SimulationCommand::None));
    QCOMPARE(static_cast<int>(inputSnapshot.request),
             static_cast<int>(SimulationRequest::ReadCurrentState));
    QVERIFY(inputSnapshot.inputs.has_value());
    QCOMPARE(static_cast<int>(inputSnapshot.inputs->mode),
             static_cast<int>(SimulationMode::StartWarmup));
    QCOMPARE(inputSnapshot.inputs->fan_AD_enabled, true);
    QCOMPARE(inputSnapshot.inputs->fan_ballast_enabled, false);

    const auto outputSnapshot = makeModelOutputSnapshot(10, 9);
    exchange.publishModelOutputSnapshot(outputSnapshot);

    QCOMPARE(readInputUInt64(registerBank, RegisterBank::revision),
             std::uint64_t{10});
    QCOMPARE(readInputUInt64(registerBank, RegisterBank::sourceInputRevision),
             std::uint64_t{9});
    QCOMPARE(readInputDouble(registerBank, RegisterBank::T_cool),
             outputSnapshot.outputs->T_cool);
}

void TestIntegration::modelAdapterAcceptsModelInputsAndReturnsModelOutputs()
{
    emulator::model::ModelBase model;
    emulator::model::ModelInputs inputs;
    inputs.mode = SimulationMode::HotLoad;
    inputs.target_rpm = 600.0;
    inputs.target_torque_nm = 50.0;
    inputs.M_AD_target = 50.0;
    inputs.f_AD = 50.0;
    inputs.throttle_position = 0.2;
    inputs.fan_ICE_enabled = true;
    inputs.fan_AD_enabled = true;
    inputs.fan_ballast_enabled = true;
    inputs.limits.P_oil_min = -1.0;
    inputs.limits.P_oil_max = 10.0;
    inputs.limits.rpm_max_lapping = 5000.0;
    inputs.limits.rpm_max_run = 5000.0;

    QVERIFY(model.initialize());
    QVERIFY(model.setInputs(inputs));

    // Проверяем, что ModelAdapter отдаёт заполненный ModelOutputs именно в
    // основном контракте, а legacy-поля остаются согласованными с ним.
    const auto outputs = model.readOutputs();
    QVERIFY(std::isfinite(outputs.T_cool));
    QVERIFY(std::isfinite(outputs.P_oil));
    QVERIFY(std::isfinite(outputs.omega_ICE_run));
    QVERIFY(std::isfinite(outputs.T_AD));
    QVERIFY(std::isfinite(outputs.T_ballast));
    QVERIFY(std::isfinite(outputs.M_AD));
    QVERIFY(outputs.T_cool > 0.0);
    QVERIFY(outputs.P_oil >= 0.0);
    QCOMPARE(outputs.T_cool, outputs.t_cool_c);
    QCOMPARE(outputs.T_AD, outputs.t_ad_c);
    QCOMPARE(outputs.T_ballast, outputs.t_ballast_c);
    QCOMPARE(outputs.P_oil, outputs.p_oil_bar);
    QCOMPARE(outputs.M_AD, outputs.m_ad_nm);
    QVERIFY(!model.diagnostics().hasFault());
}

void TestIntegration::mainCompositionPublishesSimulationSnapshotAfterModbusWrite()
{
    QSignalSpy outputPublishedSpy(
        bridge_.get(),
        &SimulationBackendBridge::sig_outputPublished);

    // Сначала задаём дискретные входы вентиляции. Это тоже Modbus-запись, но
    // без команды Start она не должна публиковать выходной снимок.
    QModbusDataUnit coils(QModbusDataUnit::Coils, 0, 3);
    coils.setValues({1, 1, 1});
    waitAndCheck(client_->sendWriteRequest(coils, modbusServerAddress));

    // Основная проверка: запись holding registers должна пройти путь
    // ModbusServer -> RegisterBank -> SimulationBackendBridge ->
    // SimulationController -> RegisterBank.
    QModbusDataUnit request(QModbusDataUnit::HoldingRegisters,
                            0,
                            RegisterBank::simulationMode + 1);
    request.setValues(makeStartColdRunRequest(1));
    waitAndCheck(client_->sendWriteRequest(request, modbusServerAddress));

    if (outputPublishedSpy.count() == 0) {
        QVERIFY(outputPublishedSpy.wait(1000));
    }
    QCOMPARE(outputPublishedSpy.count(), 1);

    QModbusDataUnit stateRead(QModbusDataUnit::InputRegisters,
                              RegisterBank::state_ir,
                              1);
    auto* stateReply = client_->sendReadRequest(stateRead, modbusServerAddress);
    waitAndCheck(stateReply);
    QCOMPARE(stateReply->result().value(0),
             static_cast<quint16>(SimulationState::Running));
    stateReply->deleteLater();

    QModbusDataUnit sourceRevisionRead(QModbusDataUnit::InputRegisters,
                                       RegisterBank::sourceInputRevision,
                                       4);
    auto* sourceRevisionReply =
        client_->sendReadRequest(sourceRevisionRead, modbusServerAddress);
    waitAndCheck(sourceRevisionReply);
    QCOMPARE(readUInt64(sourceRevisionReply->result()), std::uint64_t{1});
    sourceRevisionReply->deleteLater();

    QModbusDataUnit outputRevisionRead(QModbusDataUnit::InputRegisters,
                                       RegisterBank::revision,
                                       4);
    auto* outputRevisionReply =
        client_->sendReadRequest(outputRevisionRead, modbusServerAddress);
    waitAndCheck(outputRevisionReply);
    QVERIFY(readUInt64(outputRevisionReply->result()) > 0);
    outputRevisionReply->deleteLater();
}

void TestIntegration::clientRunsFullModeSwitchingLifecycleWithoutLimitViolation()
{
    QSignalSpy outputPublishedSpy(
        bridge_.get(),
        &SimulationBackendBridge::sig_outputPublished);

    QModbusDataUnit coils(QModbusDataUnit::Coils, 0, 3);
    coils.setValues({1, 1, 1});
    waitAndCheck(client_->sendWriteRequest(coils, modbusServerAddress));

    int expectedPublications = 0;
    std::uint64_t revision = 1;

    // Стартуем модель через ClientInputSnapshot: команда Start и первый режим
    // ColdRun приходят из holding registers.
    writeClientInputSnapshot(revision,
                             SimulationCommand::Start,
                             SimulationRequest::ReadCurrentState,
                             SimulationMode::ColdRun);
    waitForPublishedCount(outputPublishedSpy, ++expectedPublications);

    auto inputSnapshot = registerBank_->RegBankSendInfo();
    QCOMPARE(inputSnapshot.revision, revision);
    QVERIFY(inputSnapshot.inputs.has_value());
    QCOMPARE(static_cast<int>(inputSnapshot.inputs->mode),
             static_cast<int>(SimulationMode::ColdRun));

    std::vector<PublishedRegistersSnapshot> outputs(4);
    switchModeAndReadSnapshot(outputPublishedSpy,
                              expectedPublications,
                              ++revision,
                              SimulationMode::ColdRun,
                              outputs[0]);
    switchModeAndReadSnapshot(outputPublishedSpy,
                              expectedPublications,
                              revision,
                              SimulationMode::StartWarmup,
                              outputs[1]);
    switchModeAndReadSnapshot(outputPublishedSpy,
                              expectedPublications,
                              revision,
                              SimulationMode::HotNoLoad,
                              outputs[2]);
    switchModeAndReadSnapshot(outputPublishedSpy,
                              expectedPublications,
                              revision,
                              SimulationMode::HotLoad,
                              outputs[3]);

    QCOMPARE(outputs.size(), std::size_t{4});
    QCOMPARE(outputs[0].outputs.T_cool, 35.0);
    QCOMPARE(outputs[1].outputs.T_cool, 52.0);
    QCOMPARE(outputs[2].outputs.T_cool, 74.0);
    QCOMPARE(outputs[3].outputs.T_cool, 88.0);
    QCOMPARE(outputs[3].outputs.M_AD, 120.0);

    for (const auto& output : outputs) {
        QCOMPARE(static_cast<int>(output.state),
                 static_cast<int>(SimulationState::Running));
        QCOMPARE(static_cast<int>(output.faultCode),
                 static_cast<int>(SimulationFaultCode::None));
        QCOMPARE(output.hasFault, false);
        QCOMPARE(output.hasLimitViolations, false);
        QVERIFY(output.revision > 0);
        QVERIFY(output.modelTime > 0);
    }

    // Завершаем штатный жизненный цикл командой Stop, также через снимок.
    writeClientInputSnapshot(++revision,
                             SimulationCommand::Stop,
                             SimulationRequest::ReadCurrentState,
                             SimulationMode::HotLoad);
    waitForPublishedCount(outputPublishedSpy, ++expectedPublications);

    const auto stoppedSnapshot = readPublishedRegistersSnapshot();
    QCOMPARE(stoppedSnapshot.sourceInputRevision, revision);
    QCOMPARE(static_cast<int>(stoppedSnapshot.state),
             static_cast<int>(SimulationState::Stopped));
    QCOMPARE(stoppedSnapshot.hasFault, false);
}

void TestIntegration::clientRunsModeSwitchingLifecycleAndPublishesLimitViolation()
{
    QSignalSpy outputPublishedSpy(
        bridge_.get(),
        &SimulationBackendBridge::sig_outputPublished);

    int expectedPublications = 0;
    std::uint64_t revision = 1;

    // В этом сценарии клиент задаёт слишком низкие runtime-лимиты по оборотам.
    // Тестовая модель не падает сама: fault должен сформировать именно слой
    // simulation по результатам проверки ModelOutputs против лимитов из
    // ClientInputSnapshot.
    writeClientInputSnapshot(revision,
                             SimulationCommand::Start,
                             SimulationRequest::ReadCurrentState,
                             SimulationMode::ColdRun,
                             true);
    waitForPublishedCount(outputPublishedSpy, ++expectedPublications);

    PublishedRegistersSnapshot faultSnapshot;
    switchModeAndReadSnapshot(outputPublishedSpy,
                              expectedPublications,
                              ++revision,
                              SimulationMode::HotLoad,
                              faultSnapshot,
                              true);

    // - Модель проверила лимиты -- ошибка
    // - SimulationFaultCode::ModelAdapterFault
    // - контроллер проверил лимиты -- ошибка
    // - перезаписал код ошибки SimulationFaultCode::SimulationLimitsExceeded
    QCOMPARE(static_cast<int>(faultSnapshot.state),
             static_cast<int>(SimulationState::Fault));
    QCOMPARE(static_cast<int>(faultSnapshot.faultCode),
             static_cast<int>(SimulationFaultCode::SimulationLimitsExceeded));
    QCOMPARE(faultSnapshot.hasFault, true);
    QCOMPARE(faultSnapshot.hasLimitViolations, true);
    QCOMPARE(faultSnapshot.sourceInputRevision, revision);
    QVERIFY(faultSnapshot.outputs.omega_ICE_run > 50.0);

    // Завершаем аварийный цикл сбросом: backend видит Fault, отправляет Reset,
    // после чего опубликованный снимок возвращает состояние Stopped.
    writeClientInputSnapshot(++revision,
                             SimulationCommand::Reset,
                             SimulationRequest::ReadCurrentState,
                             SimulationMode::ColdRun,
                             false);
    waitForPublishedCount(outputPublishedSpy, ++expectedPublications);

    const auto resetSnapshot = readPublishedRegistersSnapshot();
    QCOMPARE(resetSnapshot.sourceInputRevision, revision);
    QCOMPARE(static_cast<int>(resetSnapshot.state),
             static_cast<int>(SimulationState::Stopped));
    QCOMPARE(static_cast<int>(resetSnapshot.faultCode),
             static_cast<int>(SimulationFaultCode::None));
    QCOMPARE(resetSnapshot.hasFault, false);
}

void TestIntegration::waitAndCheck(QModbusReply* reply)
{
    QVERIFY(reply);

    QSignalSpy finishedSpy(reply, &QModbusReply::finished);
    if (!reply->isFinished()) {
        QVERIFY(finishedSpy.wait(1000));
    }

    QCOMPARE(reply->error(), QModbusDevice::NoError);
}

void TestIntegration::waitForPublishedCount(
    QSignalSpy& spy,
    int expectedCount)
{
    if (spy.count() < expectedCount) {
        QVERIFY(spy.wait(1000));
    }

    QCOMPARE(spy.count(), expectedCount);
}

void TestIntegration::writeClientInputSnapshot(
    std::uint64_t revision,
    SimulationCommand command,
    SimulationRequest request,
    SimulationMode mode,
    bool violateLimits)
{
    QModbusDataUnit unit(QModbusDataUnit::HoldingRegisters,
                         0,
                         RegisterBank::simulationMode + 1);
    unit.setValues(makeLifecycleRequest(revision,
                                        command,
                                        request,
                                        mode,
                                        violateLimits));
    waitAndCheck(client_->sendWriteRequest(unit, modbusServerAddress));
}

QModbusDataUnit TestIntegration::readModbusDataUnit(
    QModbusDataUnit::RegisterType type,
    quint16 startAddress,
    quint16 valueCount)
{
    QModbusDataUnit unit(type, startAddress, valueCount);
    auto* reply = client_->sendReadRequest(unit, modbusServerAddress);
    waitAndCheck(reply);
    const auto result = reply->result();
    reply->deleteLater();
    return result;
}

PublishedRegistersSnapshot TestIntegration::readPublishedRegistersSnapshot()
{
    const auto registers = readModbusDataUnit(QModbusDataUnit::InputRegisters,
                                             0,
                                             RegisterBank::faultCode + 1);
    const auto discreteInputs = readModbusDataUnit(QModbusDataUnit::DiscreteInputs,
                                                  RegisterBank::hasFault,
                                                  2);
    const auto values = registers.values();

    PublishedRegistersSnapshot snapshot;
    snapshot.outputs.T_cool = readDoubleAt(values, RegisterBank::T_cool);
    snapshot.outputs.P_oil = readDoubleAt(values, RegisterBank::P_oil);
    snapshot.outputs.omega_ICE_prir =
        readDoubleAt(values, RegisterBank::omega_ICE_prir);
    snapshot.outputs.omega_ICE_run =
        readDoubleAt(values, RegisterBank::omega_ICE_run);
    snapshot.outputs.T_AD = readDoubleAt(values, RegisterBank::T_AD);
    snapshot.outputs.T_ballast = readDoubleAt(values, RegisterBank::T_ballast);
    snapshot.outputs.M_AD = readDoubleAt(values, RegisterBank::M_AD);
    snapshot.outputs.f_AD = readDoubleAt(values, RegisterBank::f_AD);
    snapshot.revision = readUInt64At(values, RegisterBank::revision);
    snapshot.sourceInputRevision =
        readUInt64At(values, RegisterBank::sourceInputRevision);
    snapshot.modelTime = readUInt64At(values, RegisterBank::modelTime);
    snapshot.timestampUtcMs = readUInt64At(values, RegisterBank::timestamp_ir);
    snapshot.state = static_cast<SimulationState>(
        values.at(RegisterBank::state_ir));
    snapshot.faultCode = static_cast<SimulationFaultCode>(
        values.at(RegisterBank::faultCode));
    snapshot.hasFault = discreteInputs.value(0) != 0;
    snapshot.hasLimitViolations = discreteInputs.value(1) != 0;
    return snapshot;
}

void TestIntegration::switchModeAndReadSnapshot(
    QSignalSpy& outputPublishedSpy,
    int& expectedPublications,
    std::uint64_t& revision,
    SimulationMode mode,
    PublishedRegistersSnapshot& snapshot,
    bool violateLimits)
{
    writeClientInputSnapshot(revision,
                             SimulationCommand::None,
                             SimulationRequest::None,
                             mode,
                             violateLimits);

    const auto inputSnapshot = registerBank_->RegBankSendInfo();
    QCOMPARE(inputSnapshot.revision, revision);
    QVERIFY(inputSnapshot.inputs.has_value());
    QCOMPARE(static_cast<int>(inputSnapshot.inputs->mode),
             static_cast<int>(mode));

    QTest::qWait(30);

    ++revision;
    writeClientInputSnapshot(revision,
                             SimulationCommand::None,
                             SimulationRequest::ReadCurrentState,
                             mode,
                             violateLimits);
    waitForPublishedCount(outputPublishedSpy, ++expectedPublications);

    snapshot = readPublishedRegistersSnapshot();
    QCOMPARE(snapshot.sourceInputRevision, revision);
}

QTEST_MAIN(TestIntegration)

#include "test_api.moc"

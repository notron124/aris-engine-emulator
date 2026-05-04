#ifndef SIMULATIONTYPES_HPP
#define SIMULATIONTYPES_HPP

#include <QDateTime>
#include <QMetaType>
#include <QString>
#include <QtGlobal>

#include <chrono>
#include <cstdint>
#include <optional>
#include <vector>

namespace emulator::simulation {

enum class SimulationMode : std::uint8_t {
    ColdRun,
    StartWarmup,
    HotNoLoad,
    HotLoad
};

enum class SimulationState : std::uint8_t {
    Stopped,
    Running,
    Fault
};

enum class SimulationCommand : std::uint8_t {
    None,
    Start,
    Stop,
    Reset,
    EmergencyStop
};

/**
 * @brief Запрос к слою симуляции, не являющийся командой управления.
 *
 * @note Команды управления передаются через SimulationCommand.
 *
 * @details Описывает, нужно ли вернуть состояние модели и нужно ли перед этим
 * выполнить расчёт (зависит от SimulationRunMode).
 */
enum class SimulationRequest : std::uint8_t {
    /**
     * @brief Нет запроса на действие.
     *
     * @details Контроллер принимает входной снимок и может запомнить новые
     * ModelInputs, но не обязан формировать ModelOutputSnapshot.
     *
     * Поведение в зависимости от SimulationRunMode:
     * - StepOnDemand: модельное время не продвигается.
     * - Continuous: фоновый расчёт, если он уже запущен, продолжает работать.
     */
    None,

    /**
     * @brief Вернуть текущее состояние симуляции без расчётного шага.
     *
     * @details Используется, когда backend хочет получить последний известный
     * статус, но не хочет инициировать новый расчёт.
     *
     * Поведение в зависимости от SimulationRunMode:
     * - StepOnDemand: возвращается последний накопленный снимок, модельное
     * время не продвигается.
     * - Continuous: возвращается актуальный снимок фонового расчёта.
     */
    ReadCurrentState,

    /**
     * @brief Выполнить расчёт, если режим симуляции работает по запросу, и
     * вернуть снимок выходных данных.
     *
     * @details Используется, когда backend хочет получить актуальный статус.
     *
     * Поведение в зависимости от SimulationRunMode:
     * - StepOnDemand: модельное время продвигается, возвращается актуальный
     * снимок.
     * - Continuous: возвращается актуальный снимок фонового расчёта.
     */
    StepAndRead
};

/**
 * @brief Режим выполнения расчётной модели.
 */
enum class SimulationRunMode : std::uint8_t {
    /**
     * @brief Расчёт только по запросу.
     */
    StepOnDemand,

    /**
     * @brief Непрерывный фоновый расчёт.
     */
    Continuous
};

/**
 * @brief Политика реакции на выход контролируемых параметров за лимиты.
 */
enum class LimitViolationAction : std::uint8_t {
    /**
     * @brief Только публиковать runtime-диагностику и продолжать расчёт.
     */
    ReportOnly,

    /**
     * @brief Переводить симуляцию в Fault при нарушении лимитов.
     */
    StopSimulation
};

struct SimulationConfig {
    SimulationRunMode runMode;
    std::chrono::milliseconds integrationStep; ///< dt модели
    LimitViolationAction limitViolationAction = LimitViolationAction::ReportOnly;
};

/**
 * @brief Конфигурация по умолчанию для расчёта по запросу.
 *
 * Модельное время продвигается только при обработке
 * SimulationRequest::StepAndRead.
 *
 * Внутренний шаг интегрирования 50 мс.
 */
inline constexpr SimulationConfig stepOnDemand50Config {
    SimulationRunMode::StepOnDemand,
    std::chrono::milliseconds{50}
};

/**
 * @brief Конфигурация по умолчанию для непрерывного фонового расчёта.
 *
 * После SimulationCommand::Start модель выполняется в фоне с внутренним
 * шагом интегрирования 50 мс.
 */
inline constexpr SimulationConfig continuous50Config {
    SimulationRunMode::Continuous,
    std::chrono::milliseconds{50}
};

struct SimulationLimits {
    /// Верхний предел температуры охлаждающей жидкости ДВС.
    double T_cool_max;
    /// Нижний предел давления масла ДВС.
    double P_oil_min;
    /// Верхний предел давления масла ДВС.
    double P_oil_max;
    /// Верхний предел omega_ICE_prir
    double omega_ICE_max_prir;
    /// Верхний предел omega_ICE_run
    double omega_ICE_max_run;
    /// Верхний предел температуры АД.
    double T_AD_max;
    /// Верхний предел температуры балласта.
    double T_ballast_max;
};

/**
 * @brief Набор предельных значений модели по умолчанию.
 *
 * Используется, когда backend не передал отдельные лимиты во входном снимке.
 */
inline constexpr SimulationLimits defaultLimits {
    105.0,
    1.5,
    7.0,
    200.0,
    2000.0,
    140.0,
    250.0
};

struct ModelInputs {
    SimulationMode mode = SimulationMode::ColdRun;

    double f_AD                 = 0.0;
    double M_AD_target          = 0.0;

    bool fan_ICE_enabled        = true;
    bool fan_AD_enabled         = true;
    bool fan_ballast_enabled    = true;

    SimulationLimits limits = defaultLimits;
};

struct ModelOutputs {
    double T_cool               = 0.0;
    double P_oil                = 0.0;
    double omega_ICE_prir       = 0.0;
    double omega_ICE_run        = 0.0;
    double T_AD                 = 0.0;
    double T_ballast            = 0.0;
    double M_AD                 = 0.0;
    double f_AD                 = 0.0;
};

struct ClientInputSnapshot {
    std::uint64_t revision      = 0; ///< Служебный номер снимка
    SimulationCommand command   = SimulationCommand::None;
    SimulationRequest request   = SimulationRequest::None;
    std::optional<ModelInputs> inputs;
};

/**
 * @brief Код диагностической ошибки слоя simulation.
 *
 * Значения фиксируют причину перехода в fault-состояние.
 */
enum class SimulationFaultCode : int {
    /**
     * @brief Ошибки нет.
     *
     * Используется в DiagnosticsSnapshot по умолчанию и для снимков без fault.
     */
    None = 0,

    /**
     * @brief Внутренняя ошибка оркестрации simulation.
     *
     * Например, ModelAdapter не инициализировался, отказался принять входы или
     * вернул ошибку при шаге расчёта без собственной диагностики.
     */
    InternalError = -1,

    /**
     * @brief Аварийная остановка по команде backend.
     *
     * Возникает при SimulationCommand::EmergencyStop.
     */
    EmergencyStop = 1,

    /**
     * @brief Превышены заданные лимиты симуляции, и политика требует останов.
     *
     * Контролируемые параметры модели вышли за пределы SimulationLimits
     * при LimitViolationAction::StopSimulation.
     */
    SimulationLimitsExceeded = 2,

    /**
     * @brief Ошибка, явно сообщённая расчётной моделью.
     *
     * Используется, когда ModelAdapter::diagnostics() возвращает fault.
     */
    ModelAdapterFault = 3,

    /**
     * @brief Неподдерживаемый режим запуска simulation.
     *
     * Возникает, если SimulationConfig::runMode не соответствует ни одной
     * зарегистрированной стратегии выполнения.
     */
    UnsupportedRunMode = 4
};

struct DiagnosticsSnapshot {
    SimulationFaultCode faultCode = SimulationFaultCode::None;
    QString message;

    [[nodiscard]] bool hasFault() const
    {
        return faultCode != SimulationFaultCode::None;
    }
};

struct LimitViolation {
    QString parameter;  ///< Имя контролируемого параметра
    double value = 0.0; ///< Значение контролируемого параметра
    QString relation;   ///< Отношение контролируемого параметра к лимиту
    double limit = 0.0; ///< Значение лимита контролируемого параметра
};

struct RuntimeDiagnostics {
    std::vector<LimitViolation> limitViolations;

    [[nodiscard]] bool hasLimitViolations() const
    {
        return !limitViolations.empty();
    }
};

struct ModelOutputSnapshot {
    std::uint64_t revision      = 0;
    std::uint64_t sourceInputRevision = 0;
    SimulationState state       = SimulationState::Stopped;
    std::optional<ModelOutputs> outputs;
    DiagnosticsSnapshot diagnostics;
    RuntimeDiagnostics runtimeDiagnostics;
    std::chrono::milliseconds modelTime{0};
    QDateTime timestampUtc;
};

} // namespace emulator::simulation

// Для использования типов в мета-объектной системе Qt
Q_DECLARE_METATYPE(emulator::simulation::SimulationState)
Q_DECLARE_METATYPE(emulator::simulation::SimulationFaultCode)
Q_DECLARE_METATYPE(emulator::simulation::DiagnosticsSnapshot)

#endif // SIMULATIONTYPES_HPP

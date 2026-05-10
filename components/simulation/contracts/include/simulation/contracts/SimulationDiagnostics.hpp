#ifndef SIMULATIONDIAGNOSTICS_HPP
#define SIMULATIONDIAGNOSTICS_HPP

#include <QString>

#include <vector>

namespace emulator::simulation::diagnostics {

/**
 * @brief Код диагностической ошибки слоя simulation.
 *
 * Значения фиксируют причину перехода в fault-состояние на уровне
 * orchestration-слоя, а не внутри расчётной модели.
 */
enum class SimulationFaultCode : int {
    /**
     * @brief Ошибки нет.
     */
    None = 0,

    /**
     * @brief Внутренняя ошибка оркестрации simulation.
     *
     * Например, ModelAdapter не инициализировался, отказался принять входы или
     * вернул ошибку при шаге расчёта.
     */
    InternalError = -1,

    /**
     * @brief Аварийная остановка по команде backend.
     */
    EmergencyStop = 1,

    /**
     * @brief Превышены заданные лимиты симуляции, и политика требует останов.
     */
    SimulationLimitsExceeded = 2,

    /**
     * @brief Ошибка, явно сообщённая расчётной моделью через ModelAdapter.
     */
    ModelAdapterFault = 3,

    /**
     * @brief Неподдерживаемый режим запуска simulation.
     */
    UnsupportedRunMode = 4
};

/**
 * @brief Диагностический снимок слоя simulation.
 */
struct SimulationDiagnosticsSnapshot {
    SimulationFaultCode faultCode = SimulationFaultCode::None;
    QString message;

    [[nodiscard]] bool hasFault() const noexcept
    {
        return faultCode != SimulationFaultCode::None;
    }

    [[nodiscard]] bool hasFault(SimulationFaultCode code) const noexcept
    {
        return faultCode == code;
    }
};

/**
 * @brief Описание одного нарушения контролируемого лимита.
 */
struct LimitViolation {
    QString parameter;  ///< Имя контролируемого параметра.
    double value = 0.0; ///< Значение контролируемого параметра.
    QString relation;   ///< Отношение контролируемого параметра к лимиту.
    double limit = 0.0; ///< Значение лимита контролируемого параметра.
};

/**
 * @brief Runtime-диагностика, которая может публиковаться без fault-останова.
 */
struct RuntimeDiagnostics {
    std::vector<LimitViolation> limitViolations;

    [[nodiscard]] bool hasLimitViolations() const noexcept
    {
        return !limitViolations.empty();
    }
};

} // namespace emulator::simulation::diagnostics

#endif // SIMULATIONDIAGNOSTICS_HPP

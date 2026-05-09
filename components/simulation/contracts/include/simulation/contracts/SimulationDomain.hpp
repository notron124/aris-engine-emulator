#ifndef SIMULATIONDOMAIN_HPP
#define SIMULATIONDOMAIN_HPP

#include "simulation/units/SimulationDiagnostics.hpp"

#include <chrono>
#include <cstdint>

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
    LimitViolationAction limitViolationAction;
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
    std::chrono::milliseconds{50},
    LimitViolationAction::ReportOnly
};

/**
 * @brief Конфигурация по умолчанию для непрерывного фонового расчёта.
 *
 * После SimulationCommand::Start модель выполняется в фоне с внутренним
 * шагом интегрирования 50 мс.
 */
inline constexpr SimulationConfig continuous50Config {
    SimulationRunMode::Continuous,
    std::chrono::milliseconds{50},
    LimitViolationAction::ReportOnly
};

} // namespace emulator::simulation

#endif // SIMULATIONDOMAIN_HPP

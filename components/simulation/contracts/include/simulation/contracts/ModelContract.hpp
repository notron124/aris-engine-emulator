#ifndef MODELCONTRACT_HPP
#define MODELCONTRACT_HPP

#include "simulation/contracts/SimulationDomain.hpp"
#include "simulation/contracts/ModelDiagnostics.hpp"

namespace emulator::model {

enum class ModelStateCode : std::uint8_t {
    /**
     * Модель ещё не инициализирована.
     */
    Uninitialized,

    /**
     * Модель инициализирована и готова к запуску.
     */
    Ready,

    /**
     * Модель выполняет расчёт или находится в активном расчётном состоянии.
     */
    Running,

    /**
     * Модель штатно остановлена.
     */
    Stopped,

    /**
     * Модель сообщила внутреннюю ошибку или аварийное состояние.
     */
    Fault
};

struct Limits {
    /// Верхний предел температуры охлаждающей жидкости ДВС.
    double T_cool_max = 105.0;
    /// Нижний предел давления масла ДВС.
    double P_oil_min = 1.5;
    /// Верхний предел давления масла ДВС.
    double P_oil_max = 7.0;
    /// Верхний предел omega_ICE_prir.
    double omega_ICE_max_prir = 200.0;
    /// Верхний предел omega_ICE_run.
    double omega_ICE_max_run = 2000.0;
    /// Верхний предел температуры АД.
    double T_AD_max = 140.0;
    /// Верхний предел температуры балласта.
    double T_ballast_max = 250.0;

    /// @todo Добавлено исключительно для обеспечения совместимости, исправить.
    double rpm_max_lapping, rpm_max_run;
};

/**
 * @brief Набор предельных значений модели по умолчанию.
 *
 * Используется, когда backend не передал отдельные лимиты во входном снимке.
 */
inline constexpr Limits defaultLimits {
    105.0,
    1.5,
    7.0,
    200.0,
    2000.0,
    140.0,
    250.0
};

/**
 * @todo Дополнить в соответствии с реализацией модели / контрактом SCADA.
 */
struct ModelInputs {
    ::emulator::simulation::SimulationMode mode =
        ::emulator::simulation::SimulationMode::ColdRun;

    double f_AD                 = 0.0;
    double M_AD_target          = 0.0;

    bool fan_ICE_enabled        = true;
    bool fan_AD_enabled         = true;
    bool fan_ballast_enabled    = true;

    Limits limits = defaultLimits;

    /// @todo Добавлено исключительно для обеспечения совместимости, исправить.
    double throttle_position, stator_frequency_hz, target_brake_torque_nm, t_cool_max, t_ad_max, t_ballast_max, p_oil_min;

    /// @todo Определить механизм аварийного завершения модели.
    bool emergency_stop_requested;
};

/**
 * @todo Дополнить в соответствии с реализацией модели / контрактом SCADA.
 */
struct ModelOutputs {
    double T_cool               = 0.0;
    double P_oil                = 0.0;
    double omega_ICE_prir       = 0.0;
    double omega_ICE_run        = 0.0;
    double T_AD                 = 0.0;
    double T_ballast            = 0.0;
    double M_AD                 = 0.0;
    double f_AD                 = 0.0;

    /// @todo Добавлено исключительно для обеспечения совместимости, исправить.
    double ice_rpm, t_cool_c, t_ad_c, t_ballast_c, p_oil_bar, m_ad_nm;
};

} // namespace emulator::model

#endif // MODELCONTRACT_HPP

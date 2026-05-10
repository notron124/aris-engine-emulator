// ModelBase.cpp
#include "ModelAdapter.hpp"
#include "models.hpp"

#include <chrono>
#include <cmath>
#include <algorithm>
#include <memory>

namespace emulator::model {

class Impl {
public:
    // Компоненты физической модели
    std::unique_ptr<ICE> ice;
    std::unique_ptr<AsyncMotor> motor;
    std::unique_ptr<FrequencyConverter> converter;
    std::unique_ptr<BallastResistor> ballast;
    
    // Состояние модели
    double current_time = 0.0;
    bool is_running = true;
    bool emergency_flag = false;
    int emergency_code = 0;
    
    // Целевые значения от пользователя
    double target_rpm = 0.0;           // целевые обороты ДВС, об/мин
    double target_torque_nm = 0.0;     // целевой тормозной момент АД, Н·м
    
    // Аварийные пределы (от пользователя)
    double t_cool_max = 105.0;         // максимальная температура ОЖ, °C
    double t_ad_max = 140.0;           // максимальная температура АД, °C
    double t_ballast_max = 250.0;      // максимальная температура балласта, °C
    double p_oil_min = 1.5;            // минимальное давление масла, бар
    double p_oil_max = 7.0;            // максимальное давление масла, бар
    double rpm_max_current = 2200.0;   // текущий предел оборотов, об/мин
    
    // Конструктор: создаём компоненты
    Impl()
        : ice(std::make_unique<ICE>())
        , motor(std::make_unique<AsyncMotor>())
        , converter(std::make_unique<FrequencyConverter>())
        , ballast(std::make_unique<BallastResistor>())
    {
        // Передаём номинальный момент АД в ЧП
        // M_AD_nom берётся из конструктора AsyncMotor (по умолчанию 2000.0)
        converter->set_ad_parameters(motor->get_moment());  
    }
    
    // Проверка аварийных пределов 
    void checkEmergencyLimits() {
        // Получаем текущие показания датчиков
        double rpm = ice->get_omega() * 60.0 / (2.0 * M_PI);
        double t_cool = ice->get_temperature();
        double t_ad = motor->get_temperature();
        double t_ballast = ballast->get_temperature();
        double p_oil = ice->get_oil_pressure();
        
        // Проверка по порядку приоритета (первое сработавшее фиксируем)
        if (t_cool >= t_cool_max) {
            emergency_flag = true;
            emergency_code = 1;        // Перегрев ОЖ
            is_running = false;
        }
        else if (t_ad >= t_ad_max) {
            emergency_flag = true;
            emergency_code = 2;        // Перегрев АД
            is_running = false;
        }
        else if (t_ballast >= t_ballast_max) {
            emergency_flag = true;
            emergency_code = 3;        // Перегрев балласта
            is_running = false;
        }
        else if (p_oil <= p_oil_min) {
            emergency_flag = true;
            emergency_code = 4;        // Низкое давление масла
            is_running = false;
        }
        else if (p_oil >= p_oil_max) {
            emergency_flag = true;
            emergency_code = 5;        // Высокое давление масла
            is_running = false;
        }
        else if (rpm >= rpm_max_current && rpm_max_current > 0) {
            emergency_flag = true;
            emergency_code = 6;        // Превышение оборотов
            is_running = false;
        }
    }
};

// Конструктор / Деструктор

ModelBase::ModelBase() : pimpl(std::make_unique<Impl>())
{

}

ModelBase::~ModelBase() = default;

bool ModelBase::initialize() {
    pimpl->is_running = true;
    pimpl->emergency_flag = false;
    pimpl->emergency_code = 0;
    pimpl->current_time = 0.0;
    return true;
}

bool ModelBase::reset() {
    // Пересоздаём все компоненты с начальными параметрами
    pimpl->ice = std::make_unique<ICE>();
    pimpl->motor = std::make_unique<AsyncMotor>();
    pimpl->converter = std::make_unique<FrequencyConverter>();
    pimpl->ballast = std::make_unique<BallastResistor>();
    
    // Повторно передаём номинальный момент в ЧП
    pimpl->converter->set_ad_parameters(pimpl->motor->get_moment());
    
    // Сброс состояния
    pimpl->current_time = 0.0;
    pimpl->is_running = true;
    pimpl->emergency_flag = false;
    pimpl->emergency_code = 0;
    
    return true;
}

// Управление (входные данные от пользователя)

bool ModelBase::setInputs(const ModelInputs& inputs) {
    // Сохраняем целевые значения от пользователя
    pimpl->target_rpm = inputs.target_rpm;               // об/мин
    pimpl->target_torque_nm = inputs.target_torque_nm;   // Н·м
    if (pimpl->target_torque_nm == 0.0) {
        pimpl->target_torque_nm = inputs.target_brake_torque_nm;
    }
    if (pimpl->target_torque_nm == 0.0) {
        pimpl->target_torque_nm = inputs.M_AD_target;
    }
    
    // Обновляем аварийные пределы (могут меняться через SCADA)
    pimpl->t_cool_max = inputs.limits.T_cool_max;
    pimpl->t_ad_max = inputs.limits.T_AD_max;
    pimpl->t_ballast_max = inputs.limits.T_ballast_max;
    pimpl->p_oil_min = inputs.limits.P_oil_min;
    pimpl->p_oil_max = inputs.limits.P_oil_max;
    
    // Выбираем предел оборотов в зависимости от режима
    if (inputs.mode == ::emulator::simulation::SimulationMode::ColdRun) {
        pimpl->rpm_max_current = inputs.limits.rpm_max_lapping;
    } else {
        pimpl->rpm_max_current = inputs.limits.rpm_max_run;
    }
    
    // Управление вентиляторами 
    pimpl->motor->set_fan(inputs.fan_AD_enabled);
    pimpl->ballast->set_fan(inputs.fan_ballast_enabled);
    
    // Устанавливаем целевые обороты ДВС
    pimpl->ice->set_target_n_rpm(inputs.target_rpm);
    
    // Передаём тормозной момент в ЧП (он далее пойдёт в АД)
    // Получаем текущие обороты ротора для расчёта скольжения
    const double n_rpm_rotor {pimpl->ice->get_omega() * 60.0 / (2.0 * M_PI)};
    pimpl->converter->set_target_torque(inputs.target_torque_nm, n_rpm_rotor);
    
    // Аварийная остановка по команде пользователя 
    if (inputs.emergency_stop) {
        pimpl->emergency_flag = true;
        pimpl->emergency_code = 7;   // Аварийная остановка по команде
        pimpl->is_running = false;
    }
    
    return true;
}

// Шаг моделирования

bool ModelBase::step(
    std::chrono::milliseconds /*modelTime*/,
    std::chrono::milliseconds dt)
{
    if (!pimpl->is_running || pimpl->emergency_flag) return false;
    
    double dt_sec = dt.count() / 1000.0;
    if (dt_sec <= 0.0) return true;
    
    // 1. Получаем текущий момент АД
    double M_AD = pimpl->motor->get_moment();
    
    // 2. Шаг ДВС (передаём момент АД как внешнюю нагрузку)
    //    J_AD передаётся из параметров AsyncMotor
    pimpl->ice->step(dt_sec, M_AD, 1.2);  // J_AD = 1.2 кг·м²
    
    // 3. Получаем текущую угловую скорость ДВС
    double omega_ice = pimpl->ice->get_omega();
    double n_rpm_ice = omega_ice * 60.0 / (2.0 * M_PI);
    
    // 4. Получаем синхронную скорость от ЧП (на основе заданного тормозного момента)
    double omega_sync = pimpl->converter->get_sync_omega();
    
    // 5. Шаг АД (тормозной режим)
    pimpl->motor->step(dt_sec, omega_ice, omega_sync);
    
    // 6. Расчёт мощности на балластном резисторе (ТЗ п.3.3)
    double P_ballast = pimpl->converter->calc_ballast_power(M_AD, n_rpm_ice);
    pimpl->ballast->set_power(P_ballast);
    
    // 7. Шаг балластного резистора (тепловая модель)
    pimpl->ballast->step(dt_sec);
    
    // 8. Проверка аварийных пределов
    pimpl->checkEmergencyLimits();
    
    // 9. Обновление симуляционного времени
    pimpl->current_time += dt_sec;
    
    return !pimpl->emergency_flag;
}

// Чтение выходных данных (датчики)

ModelOutputs ModelBase::readOutputs() const {
    ModelOutputs out;
    
    // 6 датчиков согласно ТЗ п.3.5.1 и п.5
    out.ice_rpm = pimpl->ice->get_omega() * 60.0 / (2.0 * M_PI);
    out.t_cool_c = pimpl->ice->get_temperature();
    out.t_ad_c = pimpl->motor->get_temperature();
    out.t_ballast_c = pimpl->ballast->get_temperature();
    out.p_oil_bar = pimpl->ice->get_oil_pressure();
    out.m_ad_nm = pimpl->motor->get_moment();

    out.omega_ICE_prir = out.ice_rpm;
    out.omega_ICE_run = out.ice_rpm;
    out.T_cool = out.t_cool_c;
    out.T_AD = out.t_ad_c;
    out.T_ballast = out.t_ballast_c;
    out.P_oil = out.p_oil_bar;
    out.M_AD = out.m_ad_nm;
    
    return out;
}

// Состояние модели

bool ModelBase::isRunning() const {
    return pimpl->is_running && !pimpl->emergency_flag;
}

bool ModelBase::isEmergency() const {
    return pimpl->emergency_flag;
}

// Полная диагностика

diagnostics::ModelDiagnosticsSnapshot
ModelBase::diagnostics() const
{
    diagnostics::ModelDiagnosticsSnapshot snap;
    
    // // Состояние модели
    // snap.is_running = isRunning();
    // snap.is_emergency = pimpl->emergency_flag;
    // snap.emergency_code = pimpl->emergency_code;
    // snap.current_time_s = pimpl->current_time;
    
    // // Данные с датчиков
    // auto out = readOutputs();
    // snap.ice_rpm = out.ice_rpm;
    // snap.t_cool_c = out.t_cool_c;
    // snap.t_ad_c = out.t_ad_c;
    // snap.t_ballast_c = out.t_ballast_c;
    // snap.p_oil_bar = out.p_oil_bar;
    // snap.m_ad_nm = out.m_ad_nm;
    
    // // Дополнительная диагностика
    // snap.target_rpm = pimpl->target_rpm;
    // snap.target_torque_nm = pimpl->target_torque_nm;
    
    if (pimpl->emergency_flag) {
        snap.faultCode = diagnostics::ModelFaultCode::Emergency;
        snap.message = QStringLiteral("Model emergency code %1").arg(pimpl->emergency_code);
    }
    return snap;
}

} 

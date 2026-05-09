#include "ModelAdapter.hpp"
#include <cmath>
#include <algorithm>

namespace emulator::simulation::model {

// ============================================================================
// Параметры модели 
// ============================================================================

static constexpr double J_ICE_ = 3.5;           // кг·м²
static constexpr double k_heat_ = 0.30;         // 
static constexpr double k_radiator_ = 450.0;    // Вт/°C
static constexpr double tau_cool_ = 40.0;       // с

static constexpr double J_AD_ = 1.2;            // кг·м²
static constexpr double k_loss_AD_ = 0.025;     // 
static constexpr double k_loss0_AD_ = 800.0;    // Вт
static constexpr double k_rad_AD_ = 150.0;      // Вт/°C
static constexpr double tau_AD_base_ = 900.0;   // с

static constexpr double k_rad_ballast_ = 100.0; // Вт/°C
static constexpr double tau_ballast_ = 600.0;   // с
static constexpr double P_ballast_nom_ = 400000.0; // Вт

static constexpr double T_amb_ = 25.0;          // °C
static constexpr double M_AD_nom_ = 3500.0;     // Н·м

// ============================================================================
// Внутреннее состояние модели
// ============================================================================

static struct ModelState {
    // Переменные состояния
    double omega_ICE = 0.0;          // рад/с
    double T_cool = T_amb_;          // °C
    double T_ad = T_amb_;            // °C
    double T_ballast = T_amb_;       // °C
    double p_oil = 1.5;              // бар
    double M_ad = 0.0;               // Н·м
    double P_ballast = 0.0;          // Вт
    
    // Управляющие воздействия
    double throttle = 0.0;
    double stator_freq_hz = 0.0;
    double target_torque = 0.0;
    bool fan_ad_enabled = true;
    bool fan_ballast_enabled = true;
    
    // Аварийные пределы (задаются через setInputs)
    double t_cool_max = 105.0;
    double t_ad_max = 140.0;
    double t_ballast_max = 250.0;
    double p_oil_min = 1.5;
    double p_oil_max = 7.0;
    double rpm_max_current = 2200.0;
    
    // Состояние модели
    bool is_running = false;     // true = модель считает, false = модель остановлена
    bool emergency_flag = false; // false = штатная остановка, true = авария
    int emergency_code = 0;      // код аварии 
    double current_time = 0.0;
} state;

// ============================================================================
// Вспомогательные функции
// ============================================================================

static double calculateDriveTorque() {
    // Упрощённая зависимость момента от положения дросселя и оборотов
    double max_torque = 5000.0;  // Н·м
    return state.throttle * max_torque;
}

static double calculateFrictionTorque() {
    // Внутреннее трение ДВС
    double rpm = state.omega_ice * 60.0 / (2.0 * M_PI);
    return 50.0 + 0.1 * rpm;  // Н·м
}

static double calculateFlowFactorAD() {
    if (!state.fan_ad_enabled) {
        return 0.3;  // Минимальный обдув
    }
    double load_factor = std::abs(state.M_ad) / M_AD_nom_;
    return 0.3 + 0.7 * load_factor;
}

static double calculateFlowFactorBallast() {
    if (!state.fan_ballast_enabled) {
        return 0.2;  // Минимальный обдув
    }
    double load_factor = state.P_ballast / P_ballast_nom_;
    return 0.2 + 0.8 * load_factor;
}

static void checkEmergencyLimits() {
    double rpm = state.omega_ice * 60.0 / (2.0 * M_PI);
    
    if (state.T_cool >= state.t_cool_max) {
        state.emergency_flag = true;
        state.emergency_code = 1;  // Перегрев ОЖ
        state.is_running = false;
    }
    else if (state.T_ad >= state.t_ad_max) {
        state.emergency_flag = true;
        state.emergency_code = 2;  // Перегрев АД
        state.is_running = false;
    }
    else if (state.T_ballast >= state.t_ballast_max) {
        state.emergency_flag = true;
        state.emergency_code = 3;  // Перегрев балласта
        state.is_running = false;
    }
    else if (state.p_oil <= state.p_oil_min) {
        state.emergency_flag = true;
        state.emergency_code = 4;  // Низкое давление масла
        state.is_running = false;
    }
    else if (state.p_oil >= state.p_oil_max) {
        state.emergency_flag = true;
        state.emergency_code = 5;  // Высокое давление масла
        state.is_running = false;
    }
    else if (rpm >= state.rpm_max_current && state.rpm_max_current > 0) {
        state.emergency_flag = true;
        state.emergency_code = 6;  // Превышение оборотов
        state.is_running = false;
    }
}

static void solveMechanics(double dt) {
    double M_drive = calculateDriveTorque();
    double M_friction = calculateFrictionTorque();
    double M_load = state.M_ad;
    
    double domega_dt = (M_drive - M_friction - M_load) / J_ICE_;
    state.omega_ice += domega_dt * dt;
    
    if (state.omega_ice < 0.0) state.omega_ice = 0.0;
    
    state.M_ad = state.target_torque;
}

static void updateThermal(double dt) {
    double P_ICE = calculateDriveTorque() * state.omega_ice;
    double omega_AD = state.omega_ice;
    double P_loss_AD = k_loss_AD_ * std::abs(state.M_ad) * omega_AD + k_loss0_AD_;
    
    double omega_sync = 2.0 * M_PI * state.stator_freq_hz / 2.0;
    state.P_ballast = std::max(0.0, state.M_ad * (state.omega_ice - omega_sync));
    
    // ОЖ
    double P_heat_cool = k_heat_ * P_ICE;
    double T_steady_cool = T_amb_ + P_heat_cool / k_radiator_;
    double alpha_cool = std::exp(-dt / tau_cool_);
    state.T_cool = T_steady_cool + (state.T_cool - T_steady_cool) * alpha_cool;
    
    // АД
    double flow_factor_AD = calculateFlowFactorAD();
    double tau_AD = tau_AD_base_ / flow_factor_AD;
    double T_steady_AD = T_amb_ + P_loss_AD / (k_rad_AD_ * flow_factor_AD);
    double alpha_AD = std::exp(-dt / tau_AD);
    state.T_ad = T_steady_AD + (state.T_ad - T_steady_AD) * alpha_AD;
    
    // Балласт
    double fan_factor = calculateFlowFactorBallast();
    double T_steady_ballast = T_amb_ + state.P_ballast / (k_rad_ballast_ * fan_factor);
    double alpha_ballast = std::exp(-dt / tau_ballast_);
    state.T_ballast = T_steady_ballast + (state.T_ballast - T_steady_ballast) * alpha_ballast;
}

static void calculateOilPressure() {
    double rpm = state.omega_ice * 60.0 / (2.0 * M_PI);
    double p_base = 1.0 + rpm / 500.0;
    double temp_factor = 1.0 - 0.005 * (state.T_cool - 80.0);
    temp_factor = std::max(0.5, std::min(1.2, temp_factor));
    state.p_oil = p_base * temp_factor;
    state.p_oil = std::max(0.5, std::min(10.0, state.p_oil));
}

// ============================================================================
// Публичные методы ModelAdapter
// ============================================================================

bool ModelAdapter::initialize() {
    // Сброс всех параметров в начальные значения
    state = ModelState{};
    state.is_running = true;
    return true;
}

bool ModelAdapter::reset() {
    state = ModelState{};
    state.is_running = true;
    return true;
}

bool ModelAdapter::setInputs(const ModelInputs& inputs) {
    state.throttle = inputs.throttle_position;
    state.stator_freq_hz = inputs.stator_frequency_hz;
    state.target_torque = inputs.target_brake_torque_nm;
    state.fan_ad_enabled = inputs.fan_ad_enabled;
    state.fan_ballast_enabled = inputs.fan_ballast_enabled;
    
    state.t_cool_max = inputs.t_cool_max;
    state.t_ad_max = inputs.t_ad_max;
    state.t_ballast_max = inputs.t_ballast_max;
    state.p_oil_min = inputs.p_oil_min;
    state.p_oil_max = inputs.p_oil_max;
    state.rpm_max_current = std::min(inputs.rpm_max_lapping, inputs.rpm_max_run);
    
    if (inputs.emergency_stop_requested) {
        state.emergency_flag = true;
        state.emergency_code = 7;
        state.is_running = false;
    }
    
    return true;
}

bool ModelAdapter::step(std::chrono::milliseconds modelTime, std::chrono::milliseconds dt) {
    if (!state.is_running || state.emergency_flag) return false;
    
    double dt_sec = dt.count() / 1000.0;
    if (dt_sec <= 0) return true;
    
    solveMechanics(dt_sec);
    updateThermal(dt_sec);
    calculateOilPressure();
    checkEmergencyLimits();
    
    state.current_time = modelTime.count() / 1000.0;
    
    return !state.emergency_flag;
}

ModelOutputs ModelAdapter::readOutputs() const {
    ModelOutputs outputs;
    outputs.ice_rpm = state.omega_ice * 60.0 / (2.0 * M_PI); // перевод в об./мин.
    outputs.t_cool_c = state.T_cool;
    outputs.t_ad_c = state.T_ad;
    outputs.t_ballast_c = state.T_ballast;
    outputs.p_oil_bar = state.p_oil;
    outputs.m_ad_nm = state.M_ad;
    return outputs;
}

bool ModelAdapter::isRunning() const {
    return state.is_running;
}

bool ModelAdapter::isEmergency() const {
    return state.emergency_flag;
}

DiagnosticsSnapshot ModelAdapter::diagnostics() const {
    DiagnosticsSnapshot snap;
    snap.is_running = state.is_running;
    snap.is_emergency = state.emergency_flag;
    snap.emergency_code = state.emergency_code;
    snap.current_time_s = state.current_time;
    
    auto outputs = readOutputs();
    snap.ice_rpm = outputs.ice_rpm;
    snap.t_cool_c = outputs.t_cool_c;
    snap.t_ad_c = outputs.t_ad_c;
    snap.t_ballast_c = outputs.t_ballast_c;
    snap.p_oil_bar = outputs.p_oil_bar;
    snap.m_ad_nm = outputs.m_ad_nm;
    
    snap.throttle_position = state.throttle;
    snap.stator_frequency_hz = state.stator_freq_hz;
    snap.fan_ad_enabled = state.fan_ad_enabled;
    snap.fan_ballast_enabled = state.fan_ballast_enabled;
    
    return snap;
}

} 
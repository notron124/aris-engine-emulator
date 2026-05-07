#include "models.hpp"
#include "cmath"
#include <algorithm>

double thermal_step(double T_prev, double T_steady, double tau, double dt) {
    // Общее уравнение для всех тепловых элементов
    return T_steady + (T_prev - T_steady) * std::exp(-dt / tau);
}

//--------------------------- ДВС ---------------------------------------------------
ICE::ICE(double J, double k_h, double k_rad, double tau, double Tamb, 
         double Tmax, double Pmin, double Pmax, double wmax_prir, double wmax_run, double M_peak)
        : J_ICE(J), omega_ICE(0), M_drive(0), M_internal_fric(50),
        T_cool(Tamb), k_heat(k_h), k_radiator(k_rad), tau_cool(tau),
        T_amb(Tamb), T_max(Tmax), P_oil(0), P_oil_min(Pmin), P_oil_max(Pmax),
        omega_max_prir(wmax_prir), omega_max_run(wmax_run), M_peak(M_peak) {}
    
void ICE::set_target_omega(double target_omega_rads, double dt) {
    omega_target = target_omega_rads;
    
    // Какой-то простейший регулятор
    double omega_error = omega_target - omega_ICE;
    constexpr int toeque_gain = 50;
    double torque_demand = toeque_gain * omega_error;
    
    // Ограничиваем моментом, который может выдать двигатель на текущих оборотах
    double max_torque = ICE::get_max_torque_at_speed(omega_ICE);
    torque_demand = std::clamp(torque_demand, 0.0, max_torque);
    
    M_drive = torque_demand;
}

double ICE::get_max_torque_at_speed(double omega_rads) const {
    // Максимальный момент зависит от текущих оборотов по ступенчатой зависимости
    double omega_rpm = omega_rads * 60/(2*M_PI);
    const double M_peak = 3500.0;
    if (omega_rpm < omega_max_run * 0.45) {
        return M_peak / 7 * (1 + 6 * (omega_rpm / (omega_max_run * 0.45)));
    } 
    if (omega_rpm < omega_max_run * 0.8) {
        return M_peak;
    }
    if (omega_rpm < omega_max_run) {
        double ratio = (omega_rpm - omega_max_run * 0.8) / (omega_max_run * 0.2);
        return M_peak * (1.0 - ratio * 0.25);
    }
    return M_peak * 0.75;
}

void ICE::step(double dt, double M_AD_torque, double J_AD) {
    // M_AD_torque приходит от АД (может быть положительным или отрицательным)
    M_internal_fric = 50.0 + 0.1 * omega_ICE;  // Какая-то зависимость трения от скорости
    double M_net = M_drive - M_internal_fric + M_AD_torque;
    double alpha = M_net / (J_ICE + J_AD);
    omega_ICE += alpha * dt;
    if (omega_ICE < 0) omega_ICE = 0;
    
    // Тепловая часть
    double P_ICE = M_drive * omega_ICE;
    double T_steady_cool = T_amb + (k_heat / k_radiator) * P_ICE;
    T_cool = thermal_step(T_cool, T_steady_cool, tau_cool, dt);
    
    // Давление масла 
    // Упрощённо растёт с оборотами, падает с температурой
    // При превышение температуры над номинальной 80 давление падает
    double temp_factor = std::max(0.1, 1.0 - (T_cool - 80)/100.0);
    // При максимальных оборотах давление близко к предельному, но не равно ему
    P_oil = (P_oil_zero_revs + omega_ICE / omega_max_run * (P_oil_max - P_oil_zero_revs)) * temp_factor;
}

bool ICE::is_limits_exceeded(bool is_in_running_mode) const {
    if (T_cool > T_max) {return true;}
    if (P_oil < P_oil_min) {return true;}
    if (P_oil > P_oil_max) {return true;}
    if (is_in_running_mode && omega_ICE > omega_max_run) {return true;}
    if (!is_in_running_mode && omega_ICE > omega_max_prir) {return true;}
    return false;
}

double ICE::get_omega() const { return omega_ICE; }
double ICE::get_temperature() const { return T_cool; }
double ICE::get_oil_pressure() const { return P_oil; }
void ICE::set_omega(double w) {
    // имитация пуска от стратера 
    omega_ICE = w;
}


//--------------------------- АД ---------------------------------------------------
AsyncMotor::AsyncMotor(double J, double k_l, double k_l0, double k_rad, double tau_base, double Tamb, double Tmax,
                       double Mnom, double a_c, double a_l)
    : J_AD(J), omega_AD(0), M_electromagnetic(0),
    k_loss(k_l), k_loss0(k_l0), T_AD(Tamb), k_rad_AD(k_rad),
    tau_AD_base(tau_base), T_amb(Tamb), T_max(Tmax), fan_enabled(true),
    a_const(a_c), a_load(a_l), M_AD_nom(Mnom), flow_factor(a_c), M_max(Mnom * M_max_factor) {}


void AsyncMotor::set_fan(bool enabled) { fan_enabled = enabled; }

void AsyncMotor::update_flow_factor() {
    if (fan_enabled) {
        double load_ratio = std::abs(M_electromagnetic) / M_AD_nom;
        flow_factor = a_const + a_load * load_ratio;
        return;
    }
    flow_factor = a_const;   
}

void AsyncMotor::step(double dt, double omega_ICE, double omega_sync) {
    omega_AD = omega_ICE;
    double slip = (omega_sync - omega_AD) / omega_sync;

    // Условная параболическая зависимость момента от скольжения
    double slip_ratio = slip / s_max;
    double denominator = 1.0 + slip_ratio * slip_ratio;
    M_electromagnetic = (2.0 * M_max * slip_ratio) / denominator;
    
    // Потери мощности
    double P_loss = k_loss * std::abs(M_electromagnetic) * omega_AD + k_loss0;
    
    // Тепловая модель
    AsyncMotor::update_flow_factor();
    double T_steady = T_amb + P_loss / (k_rad_AD * flow_factor);
    double tau_eff = tau_AD_base / flow_factor;
    T_AD = thermal_step(T_AD, T_steady, tau_eff, dt);
}

bool AsyncMotor::is_limits_exceeded() const {
    return T_AD > T_max;
}

double AsyncMotor::get_temperature() const { return T_AD; }
double AsyncMotor::get_moment() const { return M_electromagnetic;}
double AsyncMotor::get_omega() const { return omega_AD; }


//--------------------------- ЧП ---------------------------------------------------
FrequencyConverter::FrequencyConverter(uint8_t poles, double M_max_factor, double s_max)
    : p_poles(poles), M_max_factor(M_max_factor), s_max(s_max),
    omega_sync(0), target_torque(0), M_AD_nom(0) {}

void FrequencyConverter::set_ad_parameters(double M_nom) {
    M_AD_nom = M_nom;
    M_max = M_nom * M_max_factor;
}

void FrequencyConverter::set_target_torque(double torque_request, double omega_rotor) {
    target_torque = std::clamp(torque_request, -M_max, M_max);
    
    if (std::abs(target_torque) < 1e-3) {
        // Нет запроса - отключаем ПЧ
        FrequencyConverter::set_omega_sync(0);
        return;
    }
    
    // Рассчитываем скольжение для получения нужного момента
    double M_req_abs = std::abs(target_torque);
    double sqrt_term = std::sqrt(M_max * M_max - M_req_abs * M_req_abs);
    double slip_abs = (M_max + sqrt_term) / M_req_abs * s_max;
    double slip = (target_torque > 0) ? slip_abs : -slip_abs;
    
    // Вычисляем синхронную скорость
    double omega_sync_calc = omega_rotor / (1.0 - slip);
    
    // Устанавливаем частоту
    FrequencyConverter::set_omega_sync(omega_sync_calc * p_poles / (2 * M_PI));
}

void FrequencyConverter::set_omega_sync(double omega) {
    omega_sync = omega;
}

double FrequencyConverter::calc_ballast_power(double M_AD, double omega_rotor) const {
    // Если АД работает в генераторе (торможение), мощность идёт в балласт
    // Если нет, то балласт не стоит ненагруженным
    double slip_power = M_AD * (omega_rotor - omega_sync);
    return std::max(0.0, slip_power);
}

double FrequencyConverter::get_sync_omega() const { return omega_sync; }
double FrequencyConverter::get_target_torque() const { return target_torque; }
int FrequencyConverter::get_poles() const { return p_poles; }



BallastResistor::BallastResistor(double k_rad, double tau, double Tamb, double Tmax,
                                 double Pnom, double b_c, double b_l)
    : P_ballast(0), T_ballast(Tamb), T_max(Tmax), k_rad_ballast(k_rad),
    tau_ballast(tau), T_amb(Tamb), P_nom(Pnom), fan_enabled(true),
    b_const(b_c), b_load(b_l), fan_factor(b_c) {}

void BallastResistor::set_power(double power) {
    P_ballast = power;
}

void BallastResistor::set_fan(bool enabled) { fan_enabled = enabled; }

void BallastResistor::update_fan_factor() {
    if (fan_enabled) {
        double load_ratio = P_ballast / P_nom;
        fan_factor = b_const + b_load * load_ratio;
        return;
    }
    fan_factor = b_const;
}

void BallastResistor::step(double dt) {
    BallastResistor::update_fan_factor();
    double T_steady = T_amb + P_ballast / (k_rad_ballast * fan_factor);
    T_ballast = thermal_step(T_ballast, T_steady, tau_ballast, dt);
}

bool BallastResistor::is_limits_exceeded() const {
    return T_ballast > T_max;
}

double BallastResistor::get_temperature() const { return T_ballast; }
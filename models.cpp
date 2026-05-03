#include <cmath>
#include <algorithm>

double thermal_step(double T_current, double T_steady, double tau, double dt) {
    // Общее уравнение для всех тепловых элементов
    return T_steady + (T_current - T_steady) * std::exp(-dt / tau);
}


class ICE {
    public:
        ICE(double J = 3.5, double k_h = 0.30, double k_rad = 450.0, 
            double tau = 40.0, double Tamb = 25.0, double Tmax = 105.0,
            double Pmin = 1.5, double Pmax = 7.0,
            double wmax_prir = 200.0 * (2*M_PI/60),
            double wmax_run = 2200.0 * (2*M_PI/60))
            : J_ICE(J), omega_ICE(0), M_drive(0), M_internal_fric(50),
            T_cool(Tamb), k_heat(k_h), k_radiator(k_rad), tau_cool(tau),
            T_amb(Tamb), T_max(Tmax), P_oil(0), P_oil_min(Pmin), P_oil_max(Pmax),
            omega_max_prir(wmax_prir), omega_max_run(wmax_run) {}
        
        void set_target_omega(double target_omega_rads, double dt) {
            omega_target = target_omega_rads;
            
            // Какой-то простейший регулятор
            double omega_error = omega_target - omega_ICE;
            double torque_demand = 50 * omega_error;
            
            // Ограничиваем моментом, который может выдать двигатель на текущих оборотах
            double max_torque = get_max_torque_at_speed(omega_ICE);
            torque_demand = std::clamp(torque_demand, 0.0, max_torque);
            
            M_drive = torque_demand;
        }
        
        double get_max_torque_at_speed(double omega_rads) const {
            // Получение максимального момента при текущих оборотах
            double omega_rpm = omega_rads * 60/(2*M_PI);
            const double M_peak = 3500.0;
            double m_max = 0.0;
            if (omega_rpm < omega_max_run * 0.45) {
                m_max = M_peak / 7 * (1 + 6 * (omega_rpm / (omega_max_run * 0.45)));
            } else if (omega_rpm < omega_max_run * 0.8) {
                m_max = M_peak;
            } else if (omega_rpm < omega_max_run) {
                double ratio = (omega_rpm - omega_max_run * 0.8) / (omega_max_run * 0.2);
                m_max = M_peak * (1.0 - ratio * 0.25);
            } else {
                m_max = M_peak * 0.75;
            }
            return m_max;
        }
        
        void step(double dt, double M_AD_torque) {
            // M_AD_torque приходит от АД (может быть положительным или отрицательным)
            M_internal_fric = 50.0 + 0.1 * omega_ICE;  // Какая-то зависимость трения от скорости
            double M_net = M_drive - M_internal_fric + M_AD_torque;
            double alpha = M_net / J_ICE;
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
        
        bool check_limits(bool is_in_running_mode) const {
            // Проверка аварийных пределов
            // Заменить на нормальные ошибки
            if (T_cool > T_max) {return true;}
            if (P_oil < P_oil_min) {return true;}
            if (P_oil > P_oil_max) {return true;}
            if (is_in_running_mode && omega_ICE > omega_max_run) {return true;}
            if (!is_in_running_mode && omega_ICE > omega_max_prir) {return true;}
            return false;
        }
        
        double get_omega() const { return omega_ICE; }
        double get_temperature() const { return T_cool; }
        double get_oil_pressure() const { return P_oil; }
        void set_omega(double w) {
            // имитация пуска от стратера 
            omega_ICE = w;
        }

    private:
        double J_ICE;               // момент инерции ДВС, кг·м²
        double omega_ICE;           // текущая угловая скорость, рад/с
        double omega_target = 0.0;  // целевая угловая скорость, рад/с
        double M_drive;             // внутренний момент, Н·м
        double M_internal_fric;     // внутренний момент трения, Н·м
        double M_peak;              // максимальный момент двигателя, Н·м
        
        // Тепловые параметры
        double T_cool;              // текущая температура охлаждающей жидкости, °C
        double k_heat;              // коэф. тепловыделения в охлаждающей жидкости
        double k_radiator;          // теплоотдача радиатора, Вт/°C
        double tau_cool;            // постоянная времени охлаждающей жидкости, с
        double T_amb;               // температура окр. среды, °C
        double T_max;               // максимальная температура охлаждающей жидкости, °C
        
        // Параметры давления масла
        double P_oil;                                   // текущее давление масла, бар
        double P_oil_min, P_oil_max;                    // пределы по давлению масла, бар
        static constexpr double P_oil_zero_revs = 0.5;  // давление на нулевых оборотах, бар
        
        double omega_max_prir, omega_max_run;           // пределы по оборотам, рад/с
};

class AsyncMotor {
    public:
        AsyncMotor(double J = 1.2, double k_l = 0.025, double k_l0 = 800,
                double k_rad = 150.0, double tau_base = 900.0, double Tamb = 25.0, double Tmax = 140.0,
                double Mnom = 2000.0, double a_c = 1.0, double a_l = 2.0)
            : J_AD(J), omega_AD(0), M_electromagnetic(0),
            k_loss(k_l), k_loss0(k_l0), T_AD(Tamb), k_rad_AD(k_rad),
            tau_AD_base(tau_base), T_amb(Tamb), T_max(Tmax), fan_enabled(true),
            a_const(a_c), a_load(a_l), M_AD_nom(Mnom), flow_factor(a_c), M_max(Mnom * M_max_factor) {}
        
        
        // Управление вентилятором
        void set_fan(bool enabled) { fan_enabled = enabled; }
        
        void update_flow_factor() {
            // Расчёт коэффициента обдува
            if (fan_enabled) {
                double load_ratio = std::abs(M_electromagnetic) / M_AD_nom;
                flow_factor = a_const + a_load * load_ratio;
            } else {
                flow_factor = a_const;
            }
        }
        
        void step(double dt, double omega_ICE, double omega_sync) {
            omega_AD = omega_ICE;
            double slip = (omega_sync - omega_AD) / omega_sync;

            // Условная параболическая зависимость момента от скольжения
            double slip_ratio = slip / s_max;
            double denominator = 1.0 + slip_ratio * slip_ratio;
            M_electromagnetic = (2.0 * M_max * slip_ratio) / denominator;
            
            // Потери мощности
            double P_loss = k_loss * std::abs(M_electromagnetic) * omega_AD + k_loss0;
            
            // Тепловая модель
            update_flow_factor();
            double T_steady = T_amb + P_loss / (k_rad_AD * flow_factor);
            double tau_eff = tau_AD_base / flow_factor;
            T_AD = thermal_step(T_AD, T_steady, tau_eff, dt);
        }
        
        bool check_limits() const {
            // Проверка пределов
            // Заменить на нормальные ошибки
            if (T_AD > T_max) {return true;}
            return false;
        }
        
        double get_temperature() const { return T_AD; }
        double get_moment() const { return M_electromagnetic;}
        double get_omega() const { return omega_AD; }

    private:
        double J_AD;                // момент инерции ротора, кг·м²
        double omega_AD;            // текущая угловая скорость, рад/с
        double omega_sync;          // синхронная угловая скорость от ПЧ, рад/с
        double M_electromagnetic;   // электромагнитный момент, Н·м
        double M_AD_nom;            // номинальный момент АД, Н·м
        double s_max = 0.05;        // скольжение, соответствующее максимальному моменту
        double M_max_factor = 2.2;  // отношение M_max/M_nom (перегрузочная способность)
        double M_max;               // максимальный момент АД, Н·м

        double k_loss, k_loss0;     // Коэффициенты переменных и постоянных потерь
        
        // Тепловая модель
        double T_AD;                // температура АД, °C
        double k_rad_AD;            // теплоотдача, Вт/°C
        double tau_AD_base;         // базовая постоянная времени, с
        double T_amb;               // температура охлаждающей среды, °C
        double T_max;               // максимальная температура охлаждающей жидкости, °C
        
        // Вентилятор
        bool fan_enabled;           // вентиялтор включен, флаг
        double a_const, a_load;     // константы распределение постоянного и переменного обдува
        double flow_factor;         // коэффициент обдува
        
};



class FrequencyConverter {
public:
    FrequencyConverter(int poles = 2, double M_max_factor = 2.2, double s_max = 0.05)
        : p_poles(poles), M_max_factor(M_max_factor), s_max(s_max),
          f_stator(0), omega_sync(0), target_torque(0), M_AD_nom(0) {}
    
    void set_ad_parameters(double M_nom) {
        // Установка номинальных параметров АД
        M_AD_nom = M_nom;
        M_max = M_nom * M_max_factor;
    }
    
    void set_target_torque(double torque_request, double omega_rotor) {
        target_torque = std::clamp(torque_request, -M_max, M_max);
        
        if (std::abs(target_torque) < 1e-3) {
            // Нет запроса - отключаем ПЧ
            set_frequency(0);
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
        set_frequency(omega_sync_calc * p_poles / (2 * M_PI));
    }
    
    void set_frequency(double f_hz) {
        f_stator = f_hz;
        omega_sync = 2 * M_PI * f_stator / p_poles;
    }
    
    // Расчёт мощности на балласт (рекуперация)
    double calc_ballast_power(double M_AD, double omega_rotor) const {
        // Если АД работает в генераторе (торможение), мощность идёт в балласт
        double slip_power = M_AD * (omega_rotor - omega_sync);
        return std::max(0.0, slip_power);
    }
    
    double get_sync_omega() const { return omega_sync; }
    double get_target_torque() const { return target_torque; }
    int get_poles() const { return p_poles; }
    
private:
    int p_poles;                // число пар полюсов
    double M_max_factor;        // перегрузочная способность
    double s_max;               // критическое скольжение
    double M_AD_nom;            // номинальный момент АД
    double M_max;               // максимальный момент АД
    
    double f_stator;            // текущая частота, Гц
    double omega_sync;          // синхронная скорость, рад/с
    double target_torque;       // запрошенный момент, Н·м
};

class BallastResistor {
       
    public:
        BallastResistor(double k_rad = 100.0, double tau = 600.0, double Tamb = 25.0, double Tmax = 250.0,
                        double Pnom = 800000.0, double b_c = 1.0, double b_l = 1.5)
            : P_ballast(0), T_ballast(Tamb), T_max(Tmax), k_rad_ballast(k_rad),
            tau_ballast(tau), T_amb(Tamb), P_nom(Pnom), fan_enabled(true),
            b_const(b_c), b_load(b_l), fan_factor(b_c) {}
        
        void set_power(double power) {
            // Установка мощности от ЧП
            P_ballast = power;
        }
        
        void set_fan(bool enabled) { fan_enabled = enabled; }
        
        void update_fan_factor() {
            // Расчёт коэффициента обдува
            if (fan_enabled) {
                double load_ratio = P_ballast / P_nom;
                fan_factor = b_const + b_load * load_ratio;
            } else {
                fan_factor = b_const;
            }
        }
        
        void step(double dt) {
            update_fan_factor();
            double T_steady = T_amb + P_ballast / (k_rad_ballast * fan_factor);
            T_ballast = thermal_step(T_ballast, T_steady, tau_ballast, dt);
        }
        
        bool check_limits() const {
            // Проверка пределов
            // Заменить на нормальные ошибки
            if (T_ballast > T_max) {return true;}
            return false;
        }
        
        double get_temperature() const { return T_ballast; }

    private:
        double P_ballast;           // текущая рассеиваемая мощность, Вт
        double T_ballast;           // температура, °C
        double k_rad_ballast;       // теплоотдача, Вт/°C
        double tau_ballast;         // постоянная времени, с
        double T_amb;               // температура окружающей среды, °C
        double T_max;               // максимальная температура резистора, °C
        double P_nom;               // номинальная мощность, Вт
        
        // Вентилятор
        bool fan_enabled;           // Вентиялтор включен, флаг
        double b_const, b_load;     // Константы распределение постоянного и переменного обдува
        double fan_factor;          // Коэффициент обдува
};
#ifndef MODELS_H
#define MODELS_H

#include <cmath>
#include <algorithm>
#include <cstdint>

double thermal_step(double T_prev, double T_steady, double tau, double dt);

class ICE {
    public:
        ICE(double J = 3.5, double k_h = 0.30, double k_rad = 450.0, 
            double tau = 40.0, double Tamb = 25.0, double Tmax = 105.0,
            double Pmin = 1.5, double Pmax = 7.0,
            double wmax_prir = 200.0 * (2*M_PI/60),
            double wmax_run = 2200.0 * (2*M_PI/60),
            double M_peak = 3500.0);
        
        void set_target_omega(double target_omega_rads, double dt);
        
        double get_max_torque_at_speed(double omega_rads) const;
        
        void step(double dt, double M_AD_torque, double J_AD);
        
        bool is_limits_exceeded(bool is_in_running_mode) const;
        
        double get_omega() const;
        double get_temperature() const;
        double get_oil_pressure() const;
        void set_omega(double w);

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
                double Mnom = 2000.0, double a_c = 1.0, double a_l = 2.0);
        
        
        void set_fan(bool enabled);
        
        void update_flow_factor();
        
        void step(double dt, double omega_ICE, double omega_sync);
        
        bool is_limits_exceeded() const;
        
        double get_temperature() const;
        double get_moment() const;
        double get_omega() const;

    private:
        double J_AD;                // момент инерции ротора, кг·м²
        double omega_AD;            // текущая угловая скорость, рад/с
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
        FrequencyConverter(uint8_t poles = 2, double M_max_factor = 2.2, double s_max = 0.05);
        
        void set_ad_parameters(double M_nom);
        
        void set_target_torque(double torque_request, double omega_rotor);
        
        void set_omega_sync(double omega);
        
        double calc_ballast_power(double M_AD, double omega_rotor) const;
        
        double get_sync_omega() const;
        double get_target_torque() const;
        int get_poles() const;

    private:
        uint8_t p_poles;            // число пар полюсов
        double M_max_factor;        // перегрузочная способность
        double s_max;               // критическое скольжение
        double M_AD_nom;            // номинальный момент АД
        double M_max;               // максимальный момент АД
        
        double omega_sync;          // синхронная скорость, рад/с
        double target_torque;       // запрошенный момент, Н·м
};

class BallastResistor {
       
    public:
        BallastResistor(double k_rad = 100.0, double tau = 600.0, double Tamb = 25.0, double Tmax = 250.0,
                        double Pnom = 800000.0, double b_c = 1.0, double b_l = 1.5);
        
        void set_power(double power);
        
        void set_fan(bool enabled);
        
        void update_fan_factor();

        void step(double dt);
        
        bool is_limits_exceeded() const;
        
        double get_temperature() const;

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

#endif
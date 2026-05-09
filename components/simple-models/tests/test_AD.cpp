#include <QtTest/QTest>
#include <cmath>
#include "models.hpp"

class AsyncMotorTest : public QObject {
    Q_OBJECT

private:
    const double J_AD = 1.0;
    const double k_loss = 0.2;
    const double k_loss0 = 10.0;
    const double k_rad_AD = 100.0;
    const double tau_base = 15.0;
    const double Tamb = 20.0;
    const double Tmax = 120.0;
    const double Mnom = 1000.0;
    const double a_const = 0.8;
    const double a_load = 0.4;

private slots:
    // --- Тесты конструктора ---
    void testConstructor() {
        AsyncMotor motor(J_AD, k_loss, k_loss0, k_rad_AD, tau_base, Tamb, Tmax, Mnom, a_const, a_load);
        
        QCOMPARE(motor.get_n_rpm(), 0.0);
        QCOMPARE(motor.get_temperature(), Tamb);
        QCOMPARE(motor.get_moment(), 0.0);
    }

    void testConstructorWithInitialOmega() {
        AsyncMotor motor(J_AD, k_loss, k_loss0, k_rad_AD, tau_base, Tamb, Tmax, Mnom, a_const, a_load);
        // Омега АД равна омеге ICE при step
        motor.step(0.01, 150.0, 314.0);
        
        QCOMPARE(motor.get_n_rpm(), 150.0 * 60 / (2 * M_PI));
    }

    // --- Тесты set_fan ---
    void testFanEnabledByDefault() {
        AsyncMotor motor(J_AD, k_loss, k_loss0, k_rad_AD, tau_base, Tamb, Tmax, Mnom, a_const, a_load);
        
        // По умолчанию вентилятор включен
        motor.step(0.01, 100.0, 314.0);
        double temp_fan_on = motor.get_temperature();
        
        motor.set_fan(false);
        for (int i = 0; i < 10; i++) {
            motor.step(0.01, 100.0, 314.0);
        }
        double temp_fan_off = motor.get_temperature();
        
        // При выключенном вентиляторе температура должна расти быстрее
        QVERIFY(temp_fan_off >= temp_fan_on);
    }


    // --- Тесты update_flow_factor (косвенно через step) ---
    void testFlowFactorWithFanAndLoad() {
        AsyncMotor motor(J_AD, k_loss, k_loss0, k_rad_AD, tau_base, Tamb, Tmax, Mnom, a_const, a_load);
        
        // При выключенном вентиляторе flow_factor = a_const
        motor.set_fan(false);
        for (size_t i=1; i<10000; ++i) {
            motor.step(0.1, 100.0, 314.0);
        }
        double temp_no_fan = motor.get_temperature();
        
        // При включенном вентиляторе и нагрузке охлаждение лучше
        motor.set_fan(true);
        for (size_t i=1; i<10000; ++i) {
            motor.step(0.01, 100.0, 314.0);
        }
        double temp_with_fan = motor.get_temperature();
        
        QVERIFY(temp_with_fan <= temp_no_fan + 5.0);
    }

    // --- Тесты step ---
    void testStepMomentAtDifferentSlip() {
        AsyncMotor motor(J_AD, k_loss, k_loss0, k_rad_AD, tau_base, Tamb, Tmax, Mnom, a_const, a_load);
        
        // Нулевое скольжение - момент должеy быть 0
        motor.step(0.01, 314.0, 314.0);
        double moment_zero_slip = motor.get_moment();
        QVERIFY(std::abs(moment_zero_slip) < 0.001);
        
        // Положительное скольжение - двигательный режим
        motor.step(0.01, 300.0, 314.0);
        double moment_motor = motor.get_moment();
        QVERIFY(moment_motor > 0.0);
        
        // Отрицательное скольжение - генераторный режим
        motor.step(0.01, 320.0, 314.0);
        double moment_gen = motor.get_moment();
        QVERIFY(moment_gen < 0.0);
    }

    void testStepTemperatureStabilization() {
        AsyncMotor motor(J_AD, 0.1, k_loss0, k_rad_AD, tau_base, Tamb, Tmax, Mnom, a_const, a_load);
        
        // Долгая работа до установления теплового равновесия
        double prev_temp = motor.get_temperature();
        double prev_d_temp = 1000.0;
        double temp_diff = 0.0;
        
        for (int i = 0; i < 500; i++) {
            motor.step(0.05, 313.0, 314.0);
            double current_temp = motor.get_temperature();
            
            // Температура не должна превышать максимальную
            QVERIFY(current_temp <= Tmax + 1.0);
            
            // Проверяем, что рост замедляется
            temp_diff = current_temp - prev_temp;
            QVERIFY(temp_diff < prev_d_temp);
            prev_d_temp = temp_diff; 
            prev_temp = current_temp;
        }
    }

    void testStepWithOmegaICEVariation() {
        AsyncMotor motor(J_AD, k_loss, k_loss0, k_rad_AD, tau_base, Tamb, Tmax, Mnom, a_const, a_load);
        
        // Изменяем скорость ICE
        motor.step(0.01, 300.0, 314.0); // близко к критическому значению
        double moment_big = motor.get_moment();
        
        motor.step(0.01, 200.0, 314.0);
        double moment_small = motor.get_moment();
        
        // При увеличении скорости уменьшается скольжение -> момент должен измениться
        QVERIFY(moment_big > moment_small);
    }

    // --- Тесты предельных режимов ---
    void testLimitsNotExceeded() {
        AsyncMotor motor(J_AD, k_loss, k_loss0, k_rad_AD, tau_base, Tamb, Tmax, Mnom, a_const, a_load);
        
        // Нормальный режим
        for (int i = 0; i < 50; i++) {
            motor.step(0.05, 310.0, 314.0);
            QVERIFY(!motor.is_limits_exceeded());
        }
    }

    void testTemperatureLimitExceeded() {
        AsyncMotor motor(J_AD, k_loss, k_loss0, k_rad_AD, tau_base, 25.0, 30.0, Mnom, a_const, a_load);
        
        // Экстремальная перегрузка
        for (int i = 0; i < 100000; i++) {
            motor.step(0.1, 50.0, 314.0); // Малая скорость - практически нет охлаждения
        }
        
        // Рано или поздно температура должна превысить лимит
        bool limit_exceeded = motor.is_limits_exceeded();
        QVERIFY(limit_exceeded == true);
    }

    // --- Комплексные тесты ---
    void testCompleteSimulation() {
        AsyncMotor motor(J_AD, k_loss, k_loss0, k_rad_AD, tau_base, Tamb, Tmax, Mnom, a_const, a_load);
        
        double dt = 0.05;
        double sim_time = 2.0;
        int steps = sim_time / dt;
        
        motor.set_fan(true);
        double prev_temp = motor.get_temperature();
        
        for (int i = 0; i < steps; i++) {
            // Эмуляция разгона ICE от 0 до номинала
            double omega_ice = (i < steps/2) ? 50.0 + i * 2.0 : 300.0;
            motor.step(dt, omega_ice, 314.0);
            
            // Мониторинг температуры
            double current_temp = motor.get_temperature();
            QVERIFY(current_temp >= prev_temp);           
            prev_temp = current_temp;
        }
    }

    void testThermalInertia() {
        AsyncMotor motor(J_AD, k_loss, k_loss0, k_rad_AD, tau_base, Tamb, Tmax, Mnom, a_const, a_load);
        
        // Резкое включение нагрузки
        motor.step(0.01, 200.0, 314.0);
        double temp_step1 = motor.get_temperature();
        
        // Температура не должна измениться мгновенно
        QVERIFY(std::abs(temp_step1 - Tamb) < 10.0);
    }
};

QTEST_MAIN(AsyncMotorTest)
#include "test_AD.moc"
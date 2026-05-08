#include <QtTest/QTest>
#include <cmath>
#include "models.hpp"

class ICETest : public QObject {
    Q_OBJECT

private:
    const double J_ICE = 1.0;
    const double k_h = 100.0;
    const double k_rad = 50.0;
    const double tau = 10.0;
    const double Tamb = 20.0;
    const double Tmax = 120.0;
    const double Pmin = 1.0;
    const double Pmax = 8.0;
    const double wmax_prir = 500.0;
    const double wmax_run = 600.0;
    const double M_peak = 3500.0;

private slots:
    // --- Тесты конструктора ---
    void testConstructor() {
        ICE engine(J_ICE, k_h, k_rad, tau, Tamb, Tmax, Pmin, Pmax, wmax_prir, wmax_run, M_peak);
        
        QCOMPARE(engine.get_omega(), 0.0);
        QCOMPARE(engine.get_temperature(), Tamb);
        QCOMPARE(engine.get_oil_pressure(), 0.0);
    }

    void testConstructorWithInitialOmega() {
        ICE engine(J_ICE, k_h, k_rad, tau, Tamb, Tmax, Pmin, Pmax, wmax_prir, wmax_run, M_peak);
        engine.set_omega(100.0);
        
        QCOMPARE(engine.get_omega(), 100.0);
    }

    // --- Тесты set_target_omega ---
    void testSetTargetOmega() {
        ICE engine(J_ICE, k_h, k_rad, tau, Tamb, Tmax, Pmin, Pmax, wmax_prir, wmax_run, M_peak);
        
        engine.set_target_omega(200.0);
        engine.step(0.01, 0.0, 0.0);
        QVERIFY(engine.get_omega() > 0.0);
    }

    void testSetTargetOmegaZero() {
        ICE engine(J_ICE, k_h, k_rad, tau, Tamb, Tmax, Pmin, Pmax, wmax_prir, wmax_run, M_peak);
        engine.set_omega(100.0);
        
        engine.set_target_omega(0.0);
        engine.step(0.01, 0.0, 0.0);
        
        QVERIFY(engine.get_omega() < 100.0);
    }

    // --- Тесты get_max_torque_at_speed ---
    void testGetMaxTorqueAtSpeed() {
        ICE engine(J_ICE, k_h, k_rad, tau, Tamb, Tmax, Pmin, Pmax, wmax_prir, wmax_run, M_peak);

        // Низкие обороты
        double low_omega = (wmax_run * 0.45) / 2.0;
        double low_torque = engine.get_max_torque_at_speed(low_omega);
        QCOMPARE(medium_torque, M_peak * 4 / 7);

        // Средние обороты - пик момента
        double medium_omega = wmax_run * 0.6;
        double medium_torque = engine.get_max_torque_at_speed(medium_omega);
        QCOMPARE(medium_torque, M_peak);
        
        // Высокие обороты - момент падает
        double high_omega = wmax_run * 0.9;
        double high_torque = engine.get_max_torque_at_speed(high_omega);
        QCOMPARE(medium_torque, M_peak * 0.875);
        
        // Предельные обороты
        double over_omega = wmax_run * 1.2;
        double over_torque = engine.get_max_torque_at_speed(over_omega);
        QCOMPARE(over_torque, M_peak * 0.75);
    }

    // --- Тесты step ---
    void testStepAcceleration() {
        ICE engine(J_ICE, k_h, k_rad, tau, Tamb, Tmax, Pmin, Pmax, wmax_prir, wmax_run, M_peak);
        engine.set_target_omega(200.0);
        
        double initial_omega = engine.get_omega();
        engine.step(0.1, 0.0, 0.0);
        engine.step(0.1, 0.0, 0.0);
        engine.step(0.1, 0.0, 0.0);
        double new_omega = engine.get_omega();
        
        QVERIFY(new_omega > initial_omega);
    }

    void testStepDeceleration() {
        ICE engine(J_ICE, k_h, k_rad, tau, Tamb, Tmax, Pmin, Pmax, wmax_prir, wmax_run, M_peak);
        engine.set_omega(200.0);
        engine.set_target_omega(50.0);
        
        engine.step(0.1, 0.0, 0.0);
        engine.step(0.1, 0.0, 0.0);
        engine.step(0.1, 0.0, 0.0);
        
        QVERIFY(engine.get_omega() < 200.0);
    }

    void testStepWithADTorque() {
        ICE engine(J_ICE, k_h, k_rad, tau, Tamb, Tmax, Pmin, Pmax, wmax_prir, wmax_run, M_peak);
        engine.set_omega(100.0);
        double initial_omega = engine.get_omega();
        
        double M_AD_torque = -200.0;
        engine.step(0.1, M_AD_torque, 0.0);
        engine.step(0.1, M_AD_torque, 0.0);
        engine.step(0.1, M_AD_torque, 0.0);
        
        QVERIFY(engine.get_omega() > 0.0);
        QVERIFY(engine.get_omega() < initial_omega);
    }

    void testStepPreventsNegativeOmega() {
        ICE engine(J_ICE, k_h, k_rad, tau, Tamb, Tmax, Pmin, Pmax, wmax_prir, wmax_run, M_peak);
        engine.set_omega(10.0);
        
        engine.step(0.1, -1000.0, 0.0);
        
        QVERIFY(engine.get_omega() == 0.0);
    }

    void testStepTemperatureIncrease() {
        ICE engine(J_ICE, k_h, k_rad, tau, Tamb, Tmax, Pmin, Pmax, wmax_prir, wmax_run, M_peak);
        engine.set_target_omega(300.0);
        
        double initial_temp = engine.get_temperature();
        
        for (int i = 0; i < 100; i++) {
            engine.step(0.1, 0.0, 0.0);
        }
        
        QVERIFY(engine.get_temperature() > initial_temp);
    }

    void testLimitsNotExceeded() {
        ICE engine(J_ICE, k_h, k_rad, tau, Tamb, Tmax, Pmin, Pmax, wmax_prir, wmax_run, M_peak);
        
        QVERIFY(!engine.is_limits_exceeded(false));
        QVERIFY(!engine.is_limits_exceeded(true));
    }

    void testOmegaLimitExceededInRunningMode() {
        ICE engine(J_ICE, k_h, k_rad, tau, Tamb, Tmax, Pmin, Pmax, wmax_prir, wmax_run, M_peak);
        
        engine.set_omega(wmax_run + 100.0);
        
        QVERIFY(engine.is_limits_exceeded(true));
    }

    void testOmegaLimitExceededInPrirMode() {
        ICE engine(J_ICE, k_h, k_rad, tau, Tamb, Tmax, Pmin, Pmax, wmax_prir, wmax_run, M_peak);
        
        engine.set_omega(wmax_prir + 50.0);
        
        QVERIFY(engine.is_limits_exceeded(false));
    }

    // --- Комплексные тесты ---
    void testCompleteSimulation() {
        ICE engine(J_ICE, k_h, k_rad, tau, Tamb, Tmax, Pmin, Pmax, wmax_prir, wmax_run, M_peak);
        
        double dt = 0.05;
        double sim_time = 1.0;
        int steps = sim_time / dt;
        double prev_omega = engine.get_omega();
        double prev_temp = engine.get_temperature();
        double prev_oil_p = engine.get_oil_pressure();
        
        engine.set_target_omega(400.0);
        
        for (int i = 0; i < steps; i++) {
            engine.step(dt, 0.0, 0.0);
            
            QVERIFY(engine.get_omega() >= prev_omega);
            prev_omega = engine.get_omega();
            QVERIFY(engine.get_temperature() >= prev_temp);
            prev_temp = engine.get_temperature();
            QVERIFY(engine.get_oil_pressure() >= prev_oil_p);
            prev_oil_p = engine.get_oil_pressure();
        }
    }

    void testResponseToLoadChange() {
        ICE engine(J_ICE, k_h, k_rad, tau, Tamb, Tmax, Pmin, Pmax, wmax_prir, wmax_run, M_peak);
        
        engine.set_target_omega(300.0);
        
        for (int i = 0; i < 10; i++) {
            engine.step(0.02, 0.0, 0.0);
        }
        
        double omega_before_load = engine.get_omega();
        
        for (int i = 0; i < 10; i++) {
            engine.step(0.02, -100.0, 10.0);
        }
        
        QVERIFY(engine.get_omega() < omega_before_load);
    }
};



class AsyncMotorTest : public QObject {
    Q_OBJECT

private:
    const double J_AD = 1.0;
    const double k_loss = 0.5;
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
        
        QCOMPARE(motor.get_omega(), 0.0);
        QCOMPARE(motor.get_temperature(), Tamb);
        QCOMPARE(motor.get_moment(), 0.0);
    }

    void testConstructorWithInitialOmega() {
        AsyncMotor motor(J_AD, k_loss, k_loss0, k_rad_AD, tau_base, Tamb, Tmax, Mnom, a_const, a_load);
        // Омега АД равна омеге ICE при step
        motor.step(0.01, 150.0, 314.0);
        
        QCOMPARE(motor.get_omega(), 150.0);
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
        AsyncMotor motor(J_AD, k_loss, k_loss0, k_rad_AD, tau_base, Tamb, Tmax, Mnom, a_const, a_load);
        
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
        motor.step(0.01, 300.0, 314.0); // билзко к критическому значению
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
        AsyncMotor motor(J_AD, k_loss, k_loss0, k_rad_AD, tau_base, Tamb, Tmax, Mnom, a_const, a_load);
        
        // Экстремальная перегрузка
        for (int i = 0; i < 10000; i++) {
            motor.step(0.1, 0, 314.0); // Полная остновка - практически нет охлаждения
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



class FrequencyConverterTest : public QObject {
    Q_OBJECT

private:
    const uint8_t TEST_POLES = 4;
    const double TEST_M_MAX_FACTOR = 2.5;
    const double TEST_S_MAX = 0.05;
    const double TEST_M_NOM = 1000.0;
    const double TEST_M_MAX = TEST_M_NOM * TEST_M_MAX_FACTOR;

private slots:
    // --- Тесты конструктора ---
    void testConstructor() {
        FrequencyConverter fc(TEST_POLES, TEST_M_MAX_FACTOR, TEST_S_MAX);
        
        QCOMPARE(fc.get_poles(), TEST_POLES);
        QCOMPARE(fc.get_sync_omega(), 0.0);
        QCOMPARE(fc.get_target_torque(), 0.0);
    }

    void testConstructorWithDifferentPoles() {
        FrequencyConverter fc(2, 2.0, 0.03);
        QCOMPARE(fc.get_poles(), 2);
        
        FrequencyConverter fc2(8, 3.0, 0.07);
        QCOMPARE(fc2.get_poles(), 8);
    }

    // --- Тесты set_ad_parameters ---
    void testSetAdParameters() {
        FrequencyConverter fc(TEST_POLES, TEST_M_MAX_FACTOR, TEST_S_MAX);
        
        fc.set_ad_parameters(TEST_M_NOM);
        
        // Проверяем через установку целевого момента
        fc.set_target_torque(TEST_M_MAX * 0.8, 300.0);
        QVERIFY(fc.get_target_torque() <= TEST_M_MAX);
        QVERIFY(fc.get_target_torque() >= -TEST_M_MAX);
    }

    // --- Тесты set_omega_sync ---
    void testSetOmegaSync() {
        FrequencyConverter fc(TEST_POLES, TEST_M_MAX_FACTOR, TEST_S_MAX);
        
        double test_omega = 314.159;
        fc.set_omega_sync(test_omega);
        
        QCOMPARE(fc.get_sync_omega(), test_omega);
    }

    // --- Тесты calc_ballast_power ---
    void testCalcBallastPowerGeneratorMode() {
        FrequencyConverter fc(TEST_POLES, TEST_M_MAX_FACTOR, TEST_S_MAX);
        
        // Генераторный режим: момент отрицательный, omega_rotor > omega_sync
        double M_AD = -800.0;
        double omega_rotor = 320.0;
        double omega_sync = 300.0;
        fc.set_omega_sync(omega_sync);
        
        // slip_power = M_AD * (omega_sync - omega_rotor) = -800 * -20 = 16000
        double ballast = fc.calc_ballast_power(M_AD, omega_rotor);
        QCOMPARE(ballast, 16000.0);
    }

    void testCalcBallastPowerMotorModeNormal() {
        FrequencyConverter fc(TEST_POLES, TEST_M_MAX_FACTOR, TEST_S_MAX);
        
        // Двигательный режим: omega_rotor < omega_sync
        double M_AD = 800.0;
        double omega_rotor = 280.0;
        double omega_sync = 300.0;
        fc.set_omega_sync(omega_sync);
        
        // slip_power = 800 * (-20) = -16000, но отрицательная мощность не омжет идти в балласт
        double ballast = fc.calc_ballast_power(M_AD, omega_rotor);
        QCOMPARE(ballast, 0.0);
    }

    void testCalcBallastPowerEqualSpeeds() {
        FrequencyConverter fc(TEST_POLES, TEST_M_MAX_FACTOR, TEST_S_MAX);
        
        fc.set_omega_sync(300.0);
        
        double ballast = fc.calc_ballast_power(1000.0, 300.0);
        QCOMPARE(ballast, 0.0);
    }

    // --- Тесты комплексного взаимодействия ---
    void testCompleteWorkflow() {
        FrequencyConverter fc(TEST_POLES, TEST_M_MAX_FACTOR, TEST_S_MAX);
        fc.set_ad_parameters(TEST_M_NOM);
        
        double omega_rotor = 300.0;
        
        // 1. Запрос на разгон (двигательный режим)
        fc.set_target_torque(TEST_M_NOM * 0.7, omega_rotor);
        QVERIFY(fc.get_target_torque() > 0.0);
        QVERIFY(fc.get_sync_omega() > omega_rotor);
        
        double sync_omega_1 = fc.get_sync_omega();
        
        // 2. Увеличение скорости ротора
        omega_rotor = 320.0;
        
        // 3. Запрос на торможение (генераторный режим)
        fc.set_target_torque(-TEST_M_NOM * 0.5, omega_rotor);
        QVERIFY(fc.get_target_torque() < 0.0);
        QVERIFY(fc.get_sync_omega() < omega_rotor);
        
        double sync_omega_2 = fc.get_sync_omega();
        QVERIFY(sync_omega_2 != sync_omega_1);
        
        // 4. Проверка балластной мощности в генераторном режиме
        double ballast = fc.calc_ballast_power(fc.get_target_torque(), omega_rotor);
        QVERIFY(ballast > 0.0);
    }

    void testGetSyncOmegaAfterTorqueRequest() {
        FrequencyConverter fc(TEST_POLES, TEST_M_MAX_FACTOR, TEST_S_MAX);
        fc.set_ad_parameters(TEST_M_NOM);
        
        // Изначально синхронная скорость 0
        QCOMPARE(fc.get_sync_omega(), 0.0);
        
        fc.set_target_torque(-500.0, 300.0);
        QVERIFY(fc.get_sync_omega() < 300.0);
        
        fc.set_target_torque(500.0, 300.0);
        QVERIFY(fc.get_sync_omega() > 300.0);
    }
};
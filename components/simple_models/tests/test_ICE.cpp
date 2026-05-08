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

QTEST_MAIN(ICETest)
#include "test_ICE.moc"
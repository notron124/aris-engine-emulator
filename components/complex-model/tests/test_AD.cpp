#include "../include/total_system.h"

#include <cmath>
#include <QtTest/QTest>

constexpr double Ts  = 0.001;

class AsyncMotorTest : public QObject {
    Q_OBJECT

private:
    // const double J_AD = 1.0;
    // const double k_loss = 0.5;
    // const double k_loss0 = 10.0;
    // const double k_rad_AD = 100.0;
    // const double tau_base = 15.0;
    const double Tamb = 25.0;
    const double Tmax = 125.0;
    // const double Mnom = 1000.0;
    // const double a_const = 0.8;
    // const double a_load = 0.4;
private slots:
#define system_init() \
        Model::System system; \
        system.rtU.T_AD_max = Tmax; \
        system.initialize(); \
    system.step();

    // --- Тесты конструктора ---
    void testConstructor() {
        system_init();

        QCOMPARE(system.rtY.n_rpm_AD, 0.0);
        QCOMPARE((system.rtY.T_AD - Tamb) < 1e-5, true);
        QCOMPARE(system.rtY.M_AD, 0.0);
    }

    void testConstructorWithInitialOmega() {
        system_init()
        // Омега АД равна омеге ICE при step

        system.rtU.test_AD_omega_ICE = 150.0;
        system.rtU.test_AD_omega_sync = 314.0;

        constexpr size_t N = 0.001 / Ts;
        for (size_t i = 0; i < N; ++i) {
            system.step();
        }

        QCOMPARE(system.rtY.n_rpm_AD, 150.0 * 60 / (2 * M_PI));
    }

    // --- Тесты set_fan ---
    void testFanEnabledByDefault() {
        system_init()
        // По умолчанию вентилятор включен
        system.rtU.test_AD_omega_ICE = 100.0;
        system.rtU.test_AD_omega_sync = 314.0;
        system.step();
        double temp_fan_on = system.rtY.T_AD;

        system.rtU.ad_fan_enabled = false;
        for (size_t i = 0; i < 10; ++i) {
            system.step();
        }
        double temp_fan_off = system.rtY.T_AD;

        // При выключенном вентиляторе температура должна расти быстрее
        QVERIFY(temp_fan_off >= temp_fan_on);
    }

    // --- Тесты update_flow_factor (косвенно через step) ---
    void testFlowFactorWithFanAndLoad() {
    system_init()
        // При выключенном вентиляторе flow_factor = a_const
        system.rtU.test_AD_omega_ICE = 100.0;
        system.rtU.test_AD_omega_sync = 314.0;
        system.rtU.ad_fan_enabled = false;
        double prev_temp = system.rtY.T_AD;
        for (size_t i=1; i<10000; ++i) {
            system.step();
        }
        double delta_temp_no_fan = system.rtY.T_AD - prev_temp;

        // При включенном вентиляторе и нагрузке охлаждение лучше
        system.rtU.ad_fan_enabled = true;
        prev_temp = system.rtY.T_AD;
        for (size_t i=1; i<10000; ++i) {
            system.step();
        }
        double delta_temp_with_fan = system.rtY.T_AD - prev_temp;

        QVERIFY(delta_temp_with_fan <= delta_temp_no_fan + 5.0);
    }

    // --- Тесты step ---
    void testStepMomentAtDifferentSlip() {
    system_init()
        system.rtU.test_AD_omega_ICE = 314.0;
        system.rtU.test_AD_omega_sync = 314.0;

        // Нулевое скольжение - момент должен быть 0
        system.step();
        double moment_zero_slip = system.rtY.M_AD;
        QVERIFY(std::abs(moment_zero_slip) < 0.001);

        // Положительное скольжение - двигательный режим
        system.rtU.test_AD_omega_ICE = 300.0;
        system.step();

        double moment_motor = system.rtY.M_AD;
        QVERIFY(moment_motor > 0.0);

        system.rtU.test_AD_omega_ICE = 320.0;
        system.step();

        // Отрицательное скольжение - генераторный режим
        double moment_gen = system.rtY.M_AD;
        QVERIFY(moment_gen < 0.0);
    }

    void testStepTemperatureStabilization() {
    system_init();
    system.rtU.test_AD_omega_ICE = 313.0;
    system.rtU.test_AD_omega_sync = 314.0;
        // Долгая работа до установления теплового равновесия
        double prev_temp = system.rtY.T_AD;
        double prev_d_temp = 1000.0;
        double temp_diff = 0.0;
        for (size_t i = 0; i < 2500; ++i) {
            system.step();
            double current_temp = system.rtY.T_AD;
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
    system_init();

    system.rtU.test_AD_omega_ICE = 300.0;
    system.rtU.test_AD_omega_sync = 314.0;

    system.step();
    double moment_big = system.rtY.M_AD;

    system.rtU.test_AD_omega_ICE = 200.0;
    system.step();
    double moment_small = system.rtY.M_AD;

    // При увеличении скорости уменьшается скольжение -> момент должен измениться
    QVERIFY(moment_big > moment_small);
}
    // --- Тесты предельных режимов ---
    void testLimitsNotExceeded() {
    system_init()
    system.rtU.test_AD_omega_ICE = 310.0;
    system.rtU.test_AD_omega_sync = 314.0;

    // Нормальный режим
    for (size_t i = 0; i < 250; ++i) {
        system.step();
        QVERIFY(!system.rtY.AD_limits_exceeded);
    }
}

    void testTemperatureLimitExceeded() {
    system_init();

    system.rtU.test_AD_omega_ICE = 50.0;
    system.rtU.test_AD_omega_sync = 314.0;
    system.rtU.T_AD_max = 30;
    // Экстремальная перегрузка
    for (size_t i = 0; i < 100000 * 100; ++i) {
        system.step(); // Малая скорость - практически нет охлаждения
    }

    // Рано или поздно температура должна превысить лимит
    bool limit_exceeded = system.rtY.AD_limits_exceeded;
    QVERIFY(limit_exceeded == true);
}

    // --- Комплексные тесты ---
    void testCompleteSimulation() {
    system_init()

    double dt = 0.05;
    double sim_time = 2.0;
    int steps = sim_time / dt;

    system.rtU.ad_fan_enabled = true;
    double prev_temp = system.rtY.T_AD;

    for (size_t i = 0; i < steps; ++i) {
        // Эмуляция разгона ICE от 0 до номинала
        double omega_ice = (i < steps/2) ? 50.0 + i * 2.0 : 300.0;
        for (size_t j = 0; j < 0.05 / 0.001; ++j) {
            system.rtU.test_AD_omega_ICE = omega_ice;
            system.rtU.test_AD_omega_sync = 314.0;
        }

        // Мониторинг температуры
        double current_temp = system.rtY.T_AD;
        QVERIFY(current_temp >= prev_temp);
        prev_temp = current_temp;
    }
}

    void testThermalInertia() {
    system_init()
    system.rtU.test_AD_omega_ICE = 200.0;
    system.rtU.test_AD_omega_sync = 314.0;

    // Резкое включение нагрузки
    for (size_t i = 0; i < 0.01 / 0.001; ++i) {
        system.step();
    }
    double temp_step1 = system.rtY.T_AD;

    // Температура не должна измениться мгновенно
    QVERIFY(std::abs(temp_step1 - Tamb) < 10.0);
}
};

QTEST_MAIN(AsyncMotorTest)
#include "test_AD.moc"
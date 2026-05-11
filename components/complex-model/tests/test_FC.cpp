#include "../include/total_system.h"

#include <cmath>
#include <QtTest/QTest>

constexpr double Ts  = 0.001;

class FrequencyConverterTest : public QObject {
    Q_OBJECT

private:
    const uint8_t TEST_POLES = 2;
    const double TEST_M_MAX_FACTOR = 2.2;
    const double TEST_S_MAX = 0.05;
    const double TEST_M_NOM = 3500.0;
    const double TEST_M_MAX = TEST_M_NOM * TEST_M_MAX_FACTOR;

private slots:
#define system_init() \
Model::System system; \
system.initialize(); \
system.step();

    // --- Тесты конструктора ---
    void testConstructor() {
        system_init()

        QCOMPARE(system.rtY.FC_poles, TEST_POLES);
        QCOMPARE(system.rtY.FC_omega_sync, 0.0);
        QCOMPARE(system.rtY.target_torque, 0.0);
    }

    // --- Тесты set_ad_parameters ---
    void testSetAdParameters() {
        system_init()
        // Проверяем через установку целевого момента
        system.rtU.M_AD_target = TEST_M_MAX_FACTOR * 0.8;
        system.rtU.test_FC_n_rpm_rotor = 300;

        QVERIFY(system.rtY.target_torque <= TEST_M_MAX);
        QVERIFY(system.rtY.target_torque  >= -TEST_M_MAX);
    }

    // --- Тесты set_omega_sync ---
    void testSetOmegaSync() {
    system_init()
        double test_omega = 314.159;
    system.rtU.test_FC_set_omega_sync = test_omega;
    system.step();
        QCOMPARE(system.rtY.FC_omega_sync, test_omega);
    }

    // --- Тесты calc_ballast_power ---
    void testCalcBallastPowerGeneratorMode() {
    system_init()
        // Генераторный режим: момент отрицательный, omega_rotor > omega_sync
        double M_AD = -800.0;
        double omega_rotor = 320.0;
        double omega_sync = 300.0;
    system.rtU.test_FC_set_omega_sync = omega_sync;

   // slip_power = M_AD * (omega_sync - omega_rotor)
    system.rtU.test_FC_M_AD = M_AD;
    system.rtU.test_FC_n_rpm_rotor = omega_rotor * 60 / (2 * M_PI);
    system.step();
    system.step();
    double ballast = system.rtY.ballast_power;

    QCOMPARE(ballast, 272000.0); // не 16, потому что 2 полюса по ТЗ
    }

    void testCalcBallastPowerMotorModeNormal() {
    system_init()
        // Двигательный режим: omega_rotor < omega_sync
        double M_AD = 800.0;
        double omega_rotor = 280.0;
        double omega_sync = 300.0;
        system.rtU.test_FC_set_omega_sync = omega_sync;

        // slip_power = 800 * (-20), но отрицательная мощность не может идти в балласт
        system.rtU.test_FC_M_AD = M_AD;
        system.rtU.test_FC_n_rpm_rotor = omega_rotor * 60 / (2 * M_PI);
        system.step();
        system.step();
        double ballast = system.rtY.ballast_power;
        QCOMPARE(ballast, 0.0);
    }

    void testCalcBallastPowerEqualSpeeds() {
        system_init()
        double M_AD = 1000.0;
        double omega_rotor = 300.0;
        double omega_sync = 300.0;
        system.rtU.test_FC_set_omega_sync = omega_sync;

        system.rtU.test_FC_M_AD = M_AD;
        system.rtU.test_FC_n_rpm_rotor = omega_rotor * 60 / (2 * M_PI);
        system.step();
        system.step();
        double ballast = system.rtY.ballast_power;
        QCOMPARE(ballast, 0.0);
    }

    // --- Тесты комплексного взаимодействия ---
    void testCompleteWorkflow() {
    system_init()
        double omega_rotor = 300.0;

        // 1. Запрос на разгон (двигательный режим)
        system.rtU.M_AD_target = TEST_M_NOM * 0.7;
        system.rtU.test_FC_n_rpm_rotor = omega_rotor * 60 / (2 * M_PI);
        system.step();
        QVERIFY(system.rtY.target_torque > 0.0);
        QVERIFY(system.rtY.FC_omega_sync > omega_rotor * TEST_POLES);

        double sync_omega_1 = system.rtY.FC_omega_sync;

        // 2. Увеличение скорости ротора
        omega_rotor = 320.0;

        // 3. Запрос на торможение (генераторный режим)
        system.rtU.M_AD_target = -TEST_M_NOM * 0.5;
        system.rtU.test_FC_n_rpm_rotor = omega_rotor * 60 / (2 * M_PI);
        system.step();

        QVERIFY(system.rtY.target_torque < 0.0);
        QVERIFY(system.rtY.FC_omega_sync < omega_rotor * TEST_POLES);

        double sync_omega_2 = system.rtY.FC_omega_sync;
        QVERIFY(sync_omega_2 != sync_omega_1);

        // 4. Проверка балластной мощности в генераторном режиме

        system.rtU.test_FC_M_AD = system.rtY.target_torque;
        system.rtU.test_FC_n_rpm_rotor = omega_rotor * 60 / (2 * M_PI);
        system.step();
        system.step();
        double ballast = system.rtY.ballast_power;
        QVERIFY(ballast > 0.0);
    }

    void testGetSyncOmegaAfterTorqueRequest() {
    system_init()
        // Изначально синхронная скорость 0
        QCOMPARE(system.rtY.FC_omega_sync, 0.0);

        system.rtU.M_AD_target = -500;
        system.rtU.test_FC_n_rpm_rotor = 300 * 60 / (2 * M_PI);
        system.step();
        QVERIFY(system.rtY.FC_omega_sync < 300.0);

        system.rtU.M_AD_target = 1000;
        system.rtU.test_FC_n_rpm_rotor = 300 * 60 / (2 * M_PI);
        system.step();
        QVERIFY(system.rtY.FC_omega_sync > 300.0);
    }
};

QTEST_MAIN(FrequencyConverterTest)
#include "test_FC.moc"
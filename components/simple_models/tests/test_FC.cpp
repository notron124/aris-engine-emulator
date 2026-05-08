#include <QtTest/QTest>
#include <cmath>
#include "models.hpp"

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

QTEST_MAIN(FrequencyConverterTest)
#include "test_FC.moc"
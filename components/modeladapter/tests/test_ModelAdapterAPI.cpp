#include <QtTest/QTest>
#include "ModelAdapter.hpp"

class ModbusFullTest : public QObject {
    Q_OBJECT
private slots:

    // --- BOUNDARY TESTS ---
    void testDummy() {
        QVERIFY(dummy);
    }

private:
    bool dummy = true;
};

QTEST_MAIN(ModbusFullTest)
#include "test_ModelAdapterAPI.moc"

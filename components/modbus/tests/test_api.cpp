#include <QtTest>
#include <QModbusDataUnit>
#include <QModbusReply>
#include <QModbusTcpClient>
#include <QSignalSpy>

#include "ModbusServer.hpp"

namespace {
const QString ServerAddress = QStringLiteral("127.0.0.1");
constexpr quint16 ServerPort = 15020;
constexpr int ServerUnitId = 1;
constexpr int RequestTimeoutMs = 5000;
}

class TestModbusApi : public QObject {
    Q_OBJECT

private slots:
    void writeAndReadHoldingRegister();
};

void TestModbusApi::writeAndReadHoldingRegister()
{
    emulator::modbus::ModbusServer server(nullptr);
    QVERIFY2(server.start(ServerAddress, ServerPort), "Failed to start Modbus TCP server");
    QVERIFY(server.isRunning());

    QModbusTcpClient client;
    client.setConnectionParameter(QModbusDevice::NetworkAddressParameter, ServerAddress);
    client.setConnectionParameter(QModbusDevice::NetworkPortParameter, ServerPort);

    QVERIFY2(client.connectDevice(), qPrintable(client.errorString()));
    QTRY_COMPARE_WITH_TIMEOUT(client.state(), QModbusDevice::ConnectedState, RequestTimeoutMs);

    QModbusDataUnit writeUnit(QModbusDataUnit::HoldingRegisters, 0, 1);
    writeUnit.setValue(0, 123);

    auto* writeReply = client.sendWriteRequest(writeUnit, ServerUnitId);
    QVERIFY2(writeReply, qPrintable(client.errorString()));

    QSignalSpy writeFinished(writeReply, &QModbusReply::finished);
    if (!writeReply->isFinished()) {
        QTRY_COMPARE_WITH_TIMEOUT(writeFinished.count(), 1, RequestTimeoutMs);
    }
    QCOMPARE(writeReply->error(), QModbusDevice::NoError);
    writeReply->deleteLater();

    QModbusDataUnit readUnit(QModbusDataUnit::HoldingRegisters, 0, 1);
    auto* readReply = client.sendReadRequest(readUnit, ServerUnitId);
    QVERIFY2(readReply, qPrintable(client.errorString()));

    QSignalSpy readFinished(readReply, &QModbusReply::finished);
    if (!readReply->isFinished()) {
        QTRY_COMPARE_WITH_TIMEOUT(readFinished.count(), 1, RequestTimeoutMs);
    }

    QCOMPARE(readReply->error(), QModbusDevice::NoError);
    QCOMPARE(readReply->result().value(0), 123);
    readReply->deleteLater();

    client.disconnectDevice();
    QTRY_COMPARE_WITH_TIMEOUT(client.state(), QModbusDevice::UnconnectedState, RequestTimeoutMs);
    server.stop();
}

QTEST_MAIN(TestModbusApi)

#include "test_api.moc"

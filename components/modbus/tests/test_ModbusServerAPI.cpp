#include <QtTest/QTest>
#include <QModbusTcpClient>
#include <QtTest/QSignalSpy>
#include "ModbusServer.hpp"
#include "RegisterBankMock.hpp"

class ModbusFullTest : public QObject {
    Q_OBJECT
private slots:
    void init() {
        bank = new emulator::modbus::tests::RegisterBankMock();
        server = new emulator::modbus::ModbusServer(bank);
        server->start("127.0.0.1", 1502);

        client = new QModbusTcpClient();
        client->setConnectionParameter(QModbusDevice::NetworkAddressParameter, "127.0.0.1");
        client->setConnectionParameter(QModbusDevice::NetworkPortParameter, 1502);
        QVERIFY(client->connectDevice());

        QSignalSpy spy(client, &QModbusClient::stateChanged);
        if (client->state() != QModbusDevice::ConnectedState)
            QVERIFY(spy.wait(1000));
    }

    void cleanup() {
        if (client) {
            delete client;
        }

        if (server) {
            delete server;
        }

        if (bank) {
            delete bank;
        }
    }

    // --- COILS (Read/Write) ---
    void testCoils() {
        QModbusDataUnit writeUnit(QModbusDataUnit::Coils, 100, 2);
        writeUnit.setValues({true, false});
        auto *reply = client->sendWriteRequest(writeUnit, 1);
        waitAndCheck(reply);

        QCOMPARE(bank->readCoils(100, 2), QList<bool>({true, false}));

        QModbusDataUnit readUnit(QModbusDataUnit::Coils, 100, 2);
        auto *readReply = client->sendReadRequest(readUnit, 1);
        waitAndCheck(readReply);
        QCOMPARE(readReply->result().values()[0], 1);
        QCOMPARE(readReply->result().values()[1], 0);
    }

    // --- DISCRETE INPUTS (Read Only) ---
    void testDiscreteInputs() {
        bank->writeDiscreteInput(5, true);

        QModbusDataUnit readUnit(QModbusDataUnit::DiscreteInputs, 5, 1);
        auto *reply = client->sendReadRequest(readUnit, 1);
        waitAndCheck(reply);

        QCOMPARE(reply->result().value(0), 1u);
    }

    // --- INPUT REGISTERS (Read Only) ---
    void testInputRegisters() {
        uint16_t sensorValue = 366;
        bank->writeInputRegister(20, sensorValue);

        QModbusDataUnit readUnit(QModbusDataUnit::InputRegisters, 20, 1);
        auto *reply = client->sendReadRequest(readUnit, 1);
        waitAndCheck(reply);

        QCOMPARE(reply->result().value(0), sensorValue);
    }

    // --- HOLDING REGISTERS (Read/Write) ---
    void testHoldingRegisters() {
        QModbusDataUnit writeUnit(QModbusDataUnit::HoldingRegisters, 50, 3);
        writeUnit.setValues({10, 20, 30});

        auto *reply = client->sendWriteRequest(writeUnit, 1);
        waitAndCheck(reply);

        QCOMPARE(bank->readHoldingRegisters(50, 3), QList<uint16_t>({10, 20, 30}));
    }

    // --- BOUNDARY TESTS ---
    void testInvalidRequestSize() {
        QModbusDataUnit unit(QModbusDataUnit::HoldingRegisters, 0, 0);
        auto *reply = client->sendReadRequest(unit, 1);

        QVERIFY(reply);
        QSignalSpy spy(reply, &QModbusReply::finished);
        spy.wait(500);

        QVERIFY(reply->error() != QModbusDevice::NoError);
    }

private:
    // Хелпер для ожидания ответа
    void waitAndCheck(QModbusReply* reply) {
        QVERIFY(reply);
        QSignalSpy spy(reply, &QModbusReply::finished);
        QVERIFY(spy.wait(100));
        QCOMPARE(reply->error(), QModbusDevice::NoError);
    }

    emulator::modbus::tests::RegisterBankMock* bank;
    emulator::modbus::ModbusServer* server;
    QModbusTcpClient* client;
};

QTEST_MAIN(ModbusFullTest)
#include "test_ModbusServerAPI.moc"

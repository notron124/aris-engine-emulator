#include <QCoreApplication>
#include <QModbusTcpClient>
#include <QTimer>
#include <QDebug>
#include <qvariant.h>
#include "ModbusServer.hpp"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    emulator::modbus::ModbusServer server(nullptr, &a);
    server.start("127.0.0.1", 502);

    QModbusTcpClient *client = new QModbusTcpClient(&a);
    client->setConnectionParameter(QModbusDevice::NetworkAddressParameter, "127.0.0.1");
    client->setConnectionParameter(QModbusDevice::NetworkPortParameter, 502);

    QObject::connect(client, &QModbusClient::stateChanged, [client](QModbusDevice::State state) {
        if (state != QModbusDevice::ConnectedState) return;

        qInfo() << "Подключено. Начинаем запись...";

        // запись число 123 в Holding Register по адресу 0
        QModbusDataUnit writeUnit(QModbusDataUnit::HoldingRegisters, 0, 1);
        writeUnit.setValue(0, 123);

        auto *writeReply = client->sendWriteRequest(writeUnit, 1);
        if (!writeReply) {
            return;
        }

        QObject::connect(writeReply, &QModbusReply::finished, [client, writeReply]() {
            writeReply->deleteLater();

            if (writeReply->error() != QModbusDevice::NoError) {
                qCritical() << "Ошибка записи:" << writeReply->errorString();
                return;
            }

            qInfo() << "Запись прошла успешно!";

            QModbusDataUnit readUnit(QModbusDataUnit::HoldingRegisters, 0, 1);
            auto *readReply = client->sendReadRequest(readUnit, 1);

            if (!readReply) {
                return;
            }

            QObject::connect(readReply, &QModbusReply::finished, [client, readReply]() {
                readReply->deleteLater();

                if (!readReply || readReply->error() != QModbusDevice::NoError) {
                    // Не выводим ошибку, если мы сами закрыли соединение
                    if (readReply->error() != QModbusDevice::ReplyAbortedError) {
                        qCritical() << "Ошибка чтения:" << readReply->errorString();
                    }
                    return;
                }

                int val = readReply->result().value(0);
                qInfo() << "Прочитано значение из регистра 0:" << val;
                qInfo() << "Завершаем работу и отключаемся...";
                client->disconnectDevice();
                QCoreApplication::quit();
            });
        });
    });

    if (!client->connectDevice()) {
        qCritical() << "Ошибка подключения:" << client->errorString();
    }

    return a.exec();
}
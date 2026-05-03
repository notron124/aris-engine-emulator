#include <QCoreApplication>
#include "RegisterBank.hpp"
#include "ModbusServer.hpp"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    emulator::registerbank::RegisterBank regBank;

    emulator::modbus::ModbusServer server(&regBank, &a);
    server.start("127.0.0.1", 1502);

    return a.exec();
}
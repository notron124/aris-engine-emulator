#include <QCoreApplication>

#include "ModbusServer.hpp"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    emulator::modbus::ModbusServer server(nullptr, &a);
    if (!server.start("127.0.0.1", 502)) {
        return 1;
    }

    return a.exec();
}

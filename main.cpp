#include <QCoreApplication>

#include "ModbusServer.hpp"
#include "ModelAdapter.hpp"
#include "simulation/controller/SimulationController.hpp"
#include "simulation/exchange/SimulationBackendBridge.hpp"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // 1. Настройка сервера
    emulator::registerbank::RegisterBank registerBank;
    emulator::modbus::ModbusServer server(&registerBank, &a);

    // 2. Настройка контроллера
    /// @todo Конкретную модель передать
    auto model = emulator::model::ModelBase();
    emulator::simulation::SimulationController controller(
        model,
        emulator::simulation::continuous50Config,
        &a);

    // 3. Настройка моста
    /// @todo Нужно реализовать SimulationSnapshotExchange
    auto exchange = emulator::registerbank::SimulationSnapshotExchangeImpl(&registerBank);
    emulator::simulation::SimulationBackendBridge bridge(
        exchange,
        controller,
        &a);

    QObject::connect(
         &registerBank,
         &emulator::registerbank::RegisterBank::sig_inputSnapshotUpdated,
         &bridge,
         &emulator::simulation::SimulationBackendBridge::slot_processRequest);

    if (!server.start("127.0.0.1", 1502)) {
        return 1;
    }

    return a.exec();
}

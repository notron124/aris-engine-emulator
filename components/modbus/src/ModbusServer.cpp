#include "ModbusServer.hpp"
#include "ModbusTcpServer.hpp"
#include <QDebug>

namespace emulator::modbus {
ModbusServer::ModbusServer(emulator::registerbank::RegisterBank* regBank, QObject* parent)
    : QObject(parent)
{
    m_server = new ModbusTcpServer(regBank, this);

    connect(m_server, &QModbusServer::errorOccurred,
            this, [](QModbusDevice::Error error) {
        qWarning() << "ModbusServer error:" << error;
    });
}

ModbusServer::~ModbusServer()
{
    stop();
}

bool ModbusServer::start(const QString& address, quint16 port)
{
    m_server->setConnectionParameter(QModbusDevice::NetworkAddressParameter, address);
    m_server->setConnectionParameter(QModbusDevice::NetworkPortParameter, port);

    if (!m_server->connectDevice()) {
        qCritical() << "Failed to start Modbus TCP server:" << m_server->errorString();
        return false;
    }

    qInfo() << "Modbus TCP server started on" << address << ":" << port;
    return true;
}

void ModbusServer::stop()
{
    if (m_server->state() == QModbusDevice::ConnectedState) {
        m_server->disconnectDevice();
        qInfo() << "Modbus TCP server stopped";
    }
}

bool ModbusServer::isRunning() const
{
    return m_server->state() == QModbusDevice::ConnectedState;
}

}
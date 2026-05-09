#ifndef MODBUSSERVER_HPP
#define MODBUSSERVER_HPP

#include <QObject>
#include <QHostAddress>
#include "RegisterBank.hpp"

namespace emulator::modbus {

class ModbusTcpServer;

class ModbusServer : public QObject {
    Q_OBJECT
public:
    explicit ModbusServer(emulator::registerbank::RegisterBank* regBank,
                          QObject* parent = nullptr);
    ~ModbusServer() override;

    /// @brief Запустить сервер на указанном адресе и порту.
    /// @param address ip адресс, на котором будет запущен сервер.
    /// @param port порт, на котором будет запущен сервер.
    /// @return true, если запуск произведен успешно, false иначе.
    bool start(const QString& address, uint16_t port);

    /// @brief Остановить работу сервера.
    void stop();


    /// @brief Запущен ли сервер.
    /// @return true, если сервер в состоянии работы, false иначе.
    bool isRunning() const;

private:
    ModbusTcpServer* m_server;
};

}

#endif // MODBUSSERVER_HPP
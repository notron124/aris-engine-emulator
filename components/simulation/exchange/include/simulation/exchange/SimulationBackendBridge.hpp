#ifndef SIMULATIONBACKENDBRIDGE_HPP
#define SIMULATIONBACKENDBRIDGE_HPP

#include <QObject>
#include <QtGlobal>

#include <cstdint>

namespace emulator::simulation {

class SimulationController;
class SimulationSnapshotExchange;

/**
 * @class SimulationBackendBridge
 * @brief Связывает backend обмена снимками с SimulationController
 * через Qt-слот обработки запроса.
 *
 * @note RegisterBank является владельцем атомарности.
 * Он принимает сырые Modbus-регистры, применяет запись как одну операцию и
 * даёт наружу уже готовый ClientInputSnapshot.
 *
 */
class SimulationBackendBridge : public QObject {
    Q_OBJECT

public:
    explicit SimulationBackendBridge(
        SimulationSnapshotExchange& snapshotExchange,
        SimulationController& controller,
        QObject* parent = nullptr);

public slots:
    /**
     * @details После записи регистров в RegisterBank
     * должен вызываться этот слот.
     *
     * Пример:
     * @code
     * connect(registerBank,
     *         &RegisterBank::sig_inputSnapshotUpdated,
     *         simulationBridge,
     *         &SimulationBackendBridge::slot_processRequest);
     * @endcode
     */
    void slot_processRequest();

signals:
    /**
     * @brief Сигнал о том, что обработанный входной снимок не дал
     * выходных данных для публикации.
     *
     * Испускается из slot_processRequest(), если
     * SimulationController::processSnapshot(...) вернул std::nullopt.
     *
     * Типичный случай: backend передал снимок с SimulationRequest::None, то
     * есть контроллер мог обновить входные параметры, но не должен публиковать
     * новый ModelOutputSnapshot.
     *
     * @param sourceInputRevision Ревизия входного ClientInputSnapshot, который
     * был обработан без публикации выходного снимка.
     */
    void sig_noOutputProduced(
        std::uint64_t sourceInputRevision);

    /**
     * @brief Сигнал о том, что выходной снимок был опубликован через
     * SimulationSnapshotExchange.
     *
     * Испускается из slot_processRequest() после успешного вызова
     * publishModelOutputSnapshot(...).
     *
     * Это наблюдательный сигнал для тестов, логирования и диагностики. Не
     * является каналом передачи рабочих данных между Modbus-слоем и
     * SimulationController, так как данные уже переданы атомарным снимком через
     * SimulationSnapshotExchange.
     *
     * @param outputRevision Ревизия опубликованного ModelOutputSnapshot.
     * @param sourceInputRevision Ревизия входного ClientInputSnapshot, на
     * основании которого был сформирован выходной снимок.
     */
    void sig_outputPublished(
        std::uint64_t outputRevision, std::uint64_t sourceInputRevision);

private:
    SimulationSnapshotExchange& snapshotExchange_;
    SimulationController& controller_;
};

} // namespace emulator::simulation

#endif // SIMULATIONBACKENDBRIDGE_HPP

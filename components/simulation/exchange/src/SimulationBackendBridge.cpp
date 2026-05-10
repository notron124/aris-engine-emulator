#include "simulation/exchange/SimulationBackendBridge.hpp"

#include "simulation/controller/SimulationController.hpp"
#include "simulation/exchange/SimulationSnapshotExchange.hpp"

namespace emulator::simulation {

SimulationBackendBridge::SimulationBackendBridge(
    SimulationSnapshotExchange& snapshotExchange,
    SimulationController& controller,
    QObject* parent)
    : QObject(parent)
    , snapshotExchange_(snapshotExchange)
    , controller_(controller)
{
}

void SimulationBackendBridge::slot_processRequest()
{
    const auto inputSnapshot = snapshotExchange_.readClientInputSnapshot();
    const auto outputSnapshot = controller_.processSnapshot(inputSnapshot);
    if (!outputSnapshot.has_value()) {
        emit sig_noOutputProduced(inputSnapshot.revision);
        return;
    }

    snapshotExchange_.publishModelOutputSnapshot(*outputSnapshot);
    emit sig_outputPublished(outputSnapshot->revision, outputSnapshot->sourceInputRevision);
}

} // namespace emulator::simulation

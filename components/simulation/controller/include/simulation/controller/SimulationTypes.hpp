#ifndef SIMULATIONTYPES_HPP
#define SIMULATIONTYPES_HPP

#include <simulation/Contracts.h>

#include <QMetaType>

namespace emulator::simulation {

namespace exchange = ::emulator::exchange;
namespace model_diagnostics = ::emulator::model::diagnostics;
namespace simulation_diagnostics = ::emulator::simulation::diagnostics;

using ::emulator::model::ModelInputs;
using ::emulator::model::ModelOutputs;
using ::emulator::model::SimulationLimits;
using ::emulator::model::defaultLimits;

using DiagnosticsSnapshot =
    ::emulator::simulation::diagnostics::SimulationDiagnosticsSnapshot;
using LimitViolation = ::emulator::simulation::diagnostics::LimitViolation;
using RuntimeDiagnostics = ::emulator::simulation::diagnostics::RuntimeDiagnostics;
using SimulationDiagnosticsSnapshot =
    ::emulator::simulation::diagnostics::SimulationDiagnosticsSnapshot;
using SimulationFaultCode =
    ::emulator::simulation::diagnostics::SimulationFaultCode;

} // namespace emulator::simulation

Q_DECLARE_METATYPE(emulator::simulation::SimulationState)
Q_DECLARE_METATYPE(emulator::simulation::SimulationFaultCode)
Q_DECLARE_METATYPE(emulator::simulation::DiagnosticsSnapshot)

#endif // SIMULATIONTYPES_HPP

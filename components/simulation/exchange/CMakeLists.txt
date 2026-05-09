add_library(simulation-exchange
  include/simulation/exchange/SimulationBackendBridge.hpp
  include/simulation/exchange/SimulationSnapshotExchange.hpp

  src/SimulationBackendBridge.cpp
)

add_library(Emulator::Simulation::Exchange ALIAS simulation-exchange)

target_include_directories(simulation-exchange
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(simulation-exchange
  PUBLIC
    Emulator::Simulation::Contracts
  PRIVATE
    Emulator::Simulation::Controller
)

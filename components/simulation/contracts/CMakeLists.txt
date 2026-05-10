add_library(simulation-contracts
  include/simulation/Contracts.h
  include/simulation/contracts/ExchangeContract.hpp
  include/simulation/contracts/ModelContract.hpp
  include/simulation/contracts/ModelDiagnostics.hpp
  include/simulation/contracts/SimulationDiagnostics.hpp
  include/simulation/contracts/SimulationDomain.hpp
)

add_library(Emulator::Simulation::Contracts ALIAS simulation-contracts)

target_include_directories(simulation-contracts
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(simulation-contracts
  PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
)


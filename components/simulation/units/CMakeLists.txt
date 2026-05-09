add_library(simulation-units
  include/simulation/Units.h
  include/simulation/units/ExchangeContract.hpp
  include/simulation/units/ModelContract.hpp
  include/simulation/units/ModelDiagnostics.hpp
  include/simulation/units/SimulationDiagnostics.hpp
  include/simulation/units/SimulationDomain.hpp
)

add_library(Emulator::Simulation::Units ALIAS simulation-units)

target_include_directories(simulation-units
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(simulation-units
  PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
)


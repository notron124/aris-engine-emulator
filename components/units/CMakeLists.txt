add_library(units
  include/Units.h
  include/units/ExchangeContract.hpp
  include/units/ModelContract.hpp
  include/units/ModelDiagnostics.hpp
  include/units/SimulationDiagnostics.hpp
  include/units/SimulationDomain.hpp
  src/Units.cpp
)

add_library(Emulator::Units ALIAS units)

set_target_properties(units
  PROPERTIES
    AUTOMOC OFF
)

target_include_directories(units
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(units
  PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
)

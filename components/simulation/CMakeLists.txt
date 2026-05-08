add_library(simulation-controller
  include/ModelAdapter.hpp
  include/SimulationBackendBridge.hpp
  include/SimulationController.hpp
  include/SimulationSnapshotExchange.hpp
  include/SimulationTypes.hpp
  src/SimulationBackendBridge.cpp
  src/SimulationController.cpp
  src/runners/ContinuousSimulationRunner.cpp
  src/runners/ContinuousSimulationRunner.hpp
  src/runners/SimulationRunner.cpp
  src/runners/SimulationRunner.hpp
  src/runners/StepOnDemandSimulationRunner.cpp
  src/runners/StepOnDemandSimulationRunner.hpp
)

add_library(Emulator::Simulation::Controller ALIAS simulation-controller)

target_include_directories(simulation-controller
  PUBLIC
      ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(simulation-controller
  PUBLIC
      Qt${QT_VERSION_MAJOR}::Core
)

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

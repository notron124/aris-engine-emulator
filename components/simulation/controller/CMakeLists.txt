add_library(simulation-controller
  include/simulation/controller/SimulationController.hpp
  include/simulation/controller/SimulationTypes.hpp

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
    Emulator::Simulation::Units
  PRIVATE
    Emulator::Model::Adapter # использует модель, но не рассказывает о ней всем
)
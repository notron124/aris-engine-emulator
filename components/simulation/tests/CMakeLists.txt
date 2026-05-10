include("${CMAKE_CURRENT_LIST_DIR}/../../../cmake/AddTests.cmake")

add_tests(
    PREFIX simulation_unit
    LINK_LIBRARIES
        Emulator::Simulation::Controller
        Emulator::Simulation::Exchange
        Emulator::Model::Adapter
)

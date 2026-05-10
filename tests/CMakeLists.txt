include("${CMAKE_CURRENT_LIST_DIR}/../cmake/AddTests.cmake")

add_tests(
    PREFIX stand_integration
    LINK_LIBRARIES
        register-bank
        modbus-server
        Emulator::Model::Adapter
        Emulator::Simulation::Controller
        Emulator::Simulation::Exchange
)

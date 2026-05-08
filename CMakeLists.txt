cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(stand-emulator
    LANGUAGES CXX
)

option(BUILD_TESTING "Build tests" OFF)
include(CTest)

# ==============================================================================

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core Network SerialPort SerialBus)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Network SerialPort SerialBus)
qt_standard_project_setup()

# Важно сделать до add_subdirectory, чтобы тесты проиндексировались
include(CTest)

add_subdirectory(components/modbus)
add_subdirectory(components/registerbank)
add_subdirectory(components/simulation)

add_executable(aris-engine
    main.cpp
)

target_link_libraries(aris-engine
    PRIVATE
        register-bank
        modbus-server
        simulation-controller
)

# ==============================================================================

cmake_minimum_required(VERSION 3.16)
project(stand-emulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Network SerialBus Test)
qt_standard_project_setup()

# Компоненты в порядке зависимостей
option(BUILD_TESTING "Build tests" ON)
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests/unit/)
endif()

add_subdirectory(components/registerbank)
add_subdirectory(components/modbus)

add_executable(aris-engine
    src/core/main.cpp
    components/registerbank/include/RegisterBank.hpp
)

target_link_libraries(aris-engine
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Network
        Qt${QT_VERSION_MAJOR}::SerialBus
        register-bank
        modbus-server
)


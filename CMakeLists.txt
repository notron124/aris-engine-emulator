cmake_minimum_required(VERSION 3.16)
project(stand-emulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core Network SerialBus)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Network SerialBus)

# Компоненты в порядке зависимостей
# add_subdirectory(components/register_bank)
add_subdirectory(components/modbus)

add_executable(aris-engine
    src/core/main.cpp
)

target_link_libraries(aris-engine
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Network
        Qt${QT_VERSION_MAJOR}::SerialBus
        modbus-server
        #register-bank
)
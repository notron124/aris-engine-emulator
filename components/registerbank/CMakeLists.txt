cmake_minimum_required(VERSION 3.16)

set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(register-bank
  # src/ModbusServer.cpp
  src/RegisterBank.cpp
  include/RegisterBank.hpp
  # src/ModbusTcpServer.hpp
)

target_include_directories(register-bank
  PUBLIC
      ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(register-bank
  PUBLIC
      Qt${QT_VERSION_MAJOR}::Core
      # Qt${QT_VERSION_MAJOR}::Network
  PRIVATE
      # Qt${QT_VERSION_MAJOR}::SerialBus
      Emulator::Simulation::Exchange
)

if(BUILD_TESTS)
    add_subdirectory(tests)
endif()

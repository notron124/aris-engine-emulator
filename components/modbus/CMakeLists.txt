cmake_minimum_required(VERSION 3.16)

set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(modbus-server
  src/ModbusServer.cpp
  src/ModbusTcpServer.cpp
  include/ModbusServer.hpp
  src/ModbusTcpServer.hpp
)

target_include_directories(modbus-server
  PUBLIC
      ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(modbus-server
  PUBLIC
      # RegisterBank
      Qt${QT_VERSION_MAJOR}::Core
      Qt${QT_VERSION_MAJOR}::Network
  PRIVATE
      Qt${QT_VERSION_MAJOR}::SerialBus
)

option(BUILD_TESTING "Build tests" OFF)
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
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
    Qt${QT_VERSION_MAJOR}::SerialBus
)

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
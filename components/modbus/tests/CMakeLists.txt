add_executable(modbus-tests
    ModbusServer_tests.cpp
)

target_include_directories(modbus-tests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(modbus-tests
    PRIVATE
        register-bank
        modbus-server
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::SerialBus
        Qt${QT_VERSION_MAJOR}::Network
)

add_test(NAME modbus-tests COMMAND modbus-tests)
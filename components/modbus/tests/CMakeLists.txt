include("${CMAKE_CURRENT_LIST_DIR}/../../../cmake/AddTests.cmake")

add_tests(
    PREFIX modbus_unit
    LINK_LIBRARIES
        Qt${QT_VERSION_MAJOR}::SerialBus
        modbus-server
)

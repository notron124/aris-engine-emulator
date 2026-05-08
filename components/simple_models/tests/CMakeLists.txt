include("${CMAKE_CURRENT_LIST_DIR}/../../../cmake/AddTests.cmake")

add_tests(
    PREFIX simple_models_unit
    LINK_LIBRARIES
        Qt${QT_VERSION_MAJOR}::SerialBus
        simple_models
)
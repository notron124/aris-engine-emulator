include("${CMAKE_CURRENT_LIST_DIR}/../../../cmake/AddTests.cmake")

add_tests(
    PREFIX complex_model_unit
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
    LINK_LIBRARIES
        Qt${QT_VERSION_MAJOR}::SerialBus
        complex-model
)
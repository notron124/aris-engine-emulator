include("${CMAKE_CURRENT_LIST_DIR}/../../../cmake/AddTests.cmake")

add_tests(
    PREFIX simulation_unit
    LINK_LIBRARIES
        simulation-controller
)

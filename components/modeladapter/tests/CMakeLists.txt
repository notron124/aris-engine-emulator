include("${CMAKE_CURRENT_LIST_DIR}/../../../cmake/AddTests.cmake")

add_tests(
    PREFIX modeladapter_unit
    LINK_LIBRARIES
        modeladapter
)
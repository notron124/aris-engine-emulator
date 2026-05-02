include_guard(GLOBAL)

function(add_tests)
    set(options)
    set(one_value_args PREFIX)
    set(multi_value_args LINK_LIBRARIES INCLUDE_DIRECTORIES)
    cmake_parse_arguments(AUTO_TEST
        "${options}"
        "${one_value_args}"
        "${multi_value_args}"
        ${ARGN}
    )

    find_package(QT NAMES Qt6 REQUIRED COMPONENTS Test)
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Test)

    if(NOT AUTO_TEST_PREFIX)
        get_filename_component(AUTO_TEST_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}" NAME)
    endif()

    file(GLOB AUTO_TEST_SOURCES
        CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/test_*.cpp"
    )

    foreach(test_source IN LISTS AUTO_TEST_SOURCES)
        get_filename_component(test_name "${test_source}" NAME_WE)
        set(target_name "${AUTO_TEST_PREFIX}_${test_name}")

        add_executable("${target_name}" "${test_source}")

        if(AUTO_TEST_INCLUDE_DIRECTORIES)
            target_include_directories("${target_name}"
                PRIVATE
                    ${AUTO_TEST_INCLUDE_DIRECTORIES}
            )
        endif()

        target_link_libraries("${target_name}"
            PRIVATE
                Qt${QT_VERSION_MAJOR}::Test
                ${AUTO_TEST_LINK_LIBRARIES}
        )

        add_test(NAME "${target_name}" COMMAND "${target_name}")
    endforeach()
endfunction()

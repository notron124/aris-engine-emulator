add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/contracts)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/controller)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/exchange)

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

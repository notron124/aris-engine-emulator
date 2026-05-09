add_library(modeladapter
  src/ModelAdapter.cpp
  include/ModelAdapter.hpp
)

target_include_directories(modeladapter
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(modeladapter
      simple-models
      simulation-controller
      # Qt${QT_VERSION_MAJOR}::Core
)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
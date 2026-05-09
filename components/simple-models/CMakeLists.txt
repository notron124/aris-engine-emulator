add_library(simple-models
  src/models.cpp
  include/models.hpp
)

target_include_directories(simple-models
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(simple-models
      Qt${QT_VERSION_MAJOR}::Core
      Qt${QT_VERSION_MAJOR}::Network
      Qt${QT_VERSION_MAJOR}::SerialBus
)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
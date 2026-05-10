add_library(modeladapter
  src/ModelAdapter.cpp
  include/ModelAdapter.hpp
)

add_library(Emulator::Model::Adapter ALIAS modeladapter)

target_include_directories(modeladapter
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(modeladapter
  PUBLIC
    Emulator::Simulation::Contracts
  PRIVATE
    simple-models
)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

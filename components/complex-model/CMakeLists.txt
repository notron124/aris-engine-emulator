file(GLOB MODEL_SOURCES
        "src/*.cpp"
        "src/*.c"
)

add_library(complex-model
    ${MODEL_SOURCES}
    include/AsyncMotor.h
    include/FrequencyConverter.h
    include/BallastResistor.h
    include/ICE.h
)

target_include_directories(complex-model
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(complex-model
      Qt${QT_VERSION_MAJOR}::Core
      Qt${QT_VERSION_MAJOR}::Network
      Qt${QT_VERSION_MAJOR}::SerialBus
)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
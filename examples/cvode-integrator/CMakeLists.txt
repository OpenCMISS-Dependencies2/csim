cmake_minimum_required(VERSION 3.2)

set(PROJECT_NAME csim-cvode-integrator)
project(${PROJECT_NAME} VERSION 1.0.0 LANGUAGES CXX)

find_package(csim CONFIG CONFIGS csim-targets.cmake)

set(SOURCE_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/cvode-integrator.cpp
)

add_executable(${PROJECT_NAME}
    ${SOURCE_FILES}
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
    csim
)

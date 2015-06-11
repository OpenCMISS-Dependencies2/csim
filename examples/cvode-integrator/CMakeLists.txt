cmake_minimum_required(VERSION 3.2)

set(PROJECT_NAME csim-cvode-integrator)
project(${PROJECT_NAME} VERSION 1.0.0 LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(csim CONFIG CONFIGS csim-targets.cmake)
find_package(CVODE REQUIRED)

set(SOURCE_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/cvode-integrator.cpp
)

add_executable(${PROJECT_NAME}
    ${SOURCE_FILES}
)

set(PLATFORM_DEFINES "")
if (MSVC)
    set(PLATFORM_DEFINES "_USE_MATH_DEFINES")
endif ()

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
    ${PLATFORM_DEFINES}
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${CVODE_INCLUDE_DIR}
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
    csim
    ${CVODE_LIBRARIES}
)

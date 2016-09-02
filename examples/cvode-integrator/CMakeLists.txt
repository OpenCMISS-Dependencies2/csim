project(csim-cvode-integrator VERSION 1.0.0 LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(Sundials ${SUNDIALS_VERSION} CONFIG REQUIRED)

set(SOURCE_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/cvode-integrator.cpp
)

add_executable(csim-cvode-integrator 
    ${SOURCE_FILES}
)

set(PLATFORM_DEFINES "")
if (MSVC)
    set(PLATFORM_DEFINES "_USE_MATH_DEFINES")
endif ()

target_compile_definitions(csim-cvode-integrator
    PRIVATE
    ${PLATFORM_DEFINES}
)

target_link_libraries(csim-cvode-integrator
    PUBLIC
    csim
    sundials_cvode
)

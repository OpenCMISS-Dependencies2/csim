
set(CURRENT_TEST model)
set(CURRENT_CATEGORY api)
list(APPEND CSIM_TESTS ${CURRENT_TEST})
set(${CURRENT_TEST}_SRCS
  ${CURRENT_TEST}/model.cpp
  ${CURRENT_TEST}/execution.cpp
)

set(SBML_MODEL_RESOURCE "${CMAKE_CURRENT_SOURCE_DIR}/resources/BIOMD0000000020.xml")
set(CELLML_SINE_MODEL_RESOURCE "${CMAKE_CURRENT_SOURCE_DIR}/resources/sine/sin_approximations.xml")
set(CELLML_SINE_IMPORTS_MODEL_RESOURCE "${CMAKE_CURRENT_SOURCE_DIR}/resources/sine/sin_approximations_import.xml")
set(CELLML_INVALID_MODEL_RESOURCE "${CMAKE_CURRENT_SOURCE_DIR}/resources/invalid_cellml_1.0.xml")
set(CELLML_UNDERCONSTRAINED_MODEL_RESOURCE "${CMAKE_CURRENT_SOURCE_DIR}/resources/underconstrained_cellml_1.0.xml")


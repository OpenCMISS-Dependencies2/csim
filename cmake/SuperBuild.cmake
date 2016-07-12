include (ExternalProject)

set (DEPENDENCIES_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/Dependencies")
set (DEPENDENCIES_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/local")
set (DEPENDENCIES_CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX=${DEPENDENCIES_INSTALL_DIR}
    -DBUILD_TESTS=OFF
    -DPACKAGE_CONFIG_DIR=cmake
    -DCMAKE_PREFIX_PATH=${DEPENDENCIES_INSTALL_DIR}/cmake
    )

set_property (DIRECTORY PROPERTY EP_BASE ${DEPENDENCIES_PREFIX})

list (APPEND CMAKE_MODULE_PATH ${DEPENDENCIES_INSTALL_DIR}/cmake)

set (DEPENDENCIES)
set (EXTRA_CMAKE_ARGS)

list (APPEND DEPENDENCIES ep_zlib)
ExternalProject_Add (ep_zlib
  GIT_REPOSITORY https://github.com/OpenCMISS-Dependencies/zlib
  GIT_TAG devel

  UPDATE_COMMAND ""
  PATCH_COMMAND ""
  TEST_COMMAND ""

  CMAKE_ARGS ${DEPENDENCIES_CMAKE_ARGS}
  )

list (APPEND DEPENDENCIES ep_libxml2)
ExternalProject_Add (ep_libxml2
  DEPENDS ep_zlib
  GIT_REPOSITORY https://github.com/nickerso/opencmiss-libxml2
  GIT_TAG standalone-csim-build

  UPDATE_COMMAND ""
  PATCH_COMMAND ""
  TEST_COMMAND ""

  CMAKE_ARGS ${DEPENDENCIES_CMAKE_ARGS}
  )

# Despite the name, this is still the CellML API
list (APPEND DEPENDENCIES ep_libcellml)
ExternalProject_Add (ep_libcellml
  DEPENDS ep_libxml2
  GIT_REPOSITORY https://github.com/nickerso/opencmiss-libcellml
  GIT_TAG standalone-csim-building

  UPDATE_COMMAND ""
  PATCH_COMMAND ""
  TEST_COMMAND ""

  CMAKE_ARGS ${DEPENDENCIES_CMAKE_ARGS}
  )

# shouldn't need this if we can find all the config files
#list (APPEND EXTRA_CMAKE_ARGS
#  -DBOOST_ROOT=${CMAKE_CURRENT_BINARY_DIR}/Dependencies/Source/ep_boost
#  -DBoost_NO_SYSTEM_PATHS=ON)

# FIXME add to default target "all"?
#ExternalProject_Add (ep_blah
#  DEPENDS ${DEPENDENCIES}
#  SOURCE_DIR ${PROJECT_SOURCE_DIR}
#  CMAKE_ARGS -DUSE_SUPERBUILD=OFF ${EXTRA_CMAKE_ARGS}
#  INSTALL_COMMAND ""
#  BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/blah)


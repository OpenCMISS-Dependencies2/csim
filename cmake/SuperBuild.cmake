include (ExternalProject)

set (DEPENDENCIES_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/Dependencies")
set (DEPENDENCIES_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/local")
set (DEPENDENCIES_CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX=${DEPENDENCIES_INSTALL_DIR}
    -DPACKAGE_CONFIG_DIR=cmake
    -DCMAKE_PREFIX_PATH=${DEPENDENCIES_INSTALL_DIR}/cmake
    -DCMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_NO_SYSTEM_FROM_IMPORTED=YES
    -DCMAKE_POSITION_INDEPENDENT_CODE=YES # -fPIC flag - always enable
    )

set_property (DIRECTORY PROPERTY EP_BASE ${DEPENDENCIES_PREFIX})

set (DEPENDENCIES)
set (EXTRA_CMAKE_ARGS)

list (APPEND DEPENDENCIES ep_zlib)
ExternalProject_Add (ep_zlib
  GIT_REPOSITORY https://github.com/OpenCMISS-Dependencies/zlib
  GIT_TAG devel

  UPDATE_COMMAND ""
  PATCH_COMMAND ""
  BUILD_COMMAND "ninja"
  TEST_COMMAND ""

  CMAKE_GENERATOR "Ninja"
  CMAKE_ARGS ${DEPENDENCIES_CMAKE_ARGS} -DBUILD_TESTS=OFF
  )

list (APPEND DEPENDENCIES ep_libxml2)
ExternalProject_Add (ep_libxml2
  DEPENDS ep_zlib
  GIT_REPOSITORY https://github.com/nickerso/opencmiss-libxml2
  GIT_TAG standalone-csim-build

  UPDATE_COMMAND ""
  PATCH_COMMAND ""
  BUILD_COMMAND "ninja"
  TEST_COMMAND ""

  CMAKE_GENERATOR "Ninja"
  CMAKE_ARGS ${DEPENDENCIES_CMAKE_ARGS} -DBUILD_TESTS=OFF
  )

# Despite the name, this is still the CellML API
list (APPEND DEPENDENCIES ep_libcellml)
ExternalProject_Add (ep_libcellml
  DEPENDS ep_libxml2 ep_zlib
  GIT_REPOSITORY https://github.com/nickerso/opencmiss-libcellml
  GIT_TAG standalone-csim-building

  UPDATE_COMMAND ""
  PATCH_COMMAND ""
  BUILD_COMMAND "ninja"
  TEST_COMMAND ""

  CMAKE_GENERATOR "Ninja"
  CMAKE_ARGS ${DEPENDENCIES_CMAKE_ARGS} -DBUILD_TESTING=OFF
    -DENABLE_ANNOTOOLS=ON
    -DENABLE_CCGS=ON
    -DENABLE_CELEDS=OFF
    -DENABLE_CELEDS_EXPORTER=OFF
    -DENABLE_CIS=OFF
    -DENABLE_CUSES=ON
    -DENABLE_CEVAS=ON
    -DENABLE_CONTEXT=OFF
    -DENABLE_EXAMPLES=OFF
    -DENABLE_GSL_INTEGRATORS=OFF
    -DENABLE_MALAES=ON
    -DENABLE_MOFS=OFF
    -DENABLE_RDF=OFF
    -DENABLE_SPROS=OFF
    -DENABLE_SRUS=OFF
    -DENABLE_TELICEMS=OFF
    -DENABLE_VACSS=OFF
    -DENABLE_CGRS=OFF
  )

#list (APPEND DEPENDENCIES ep_llvm)
#ExternalProject_Add (ep_llvm
#  DEPENDS ep_libxml2 ep_libcellml
#  GIT_REPOSITORY https://github.com/OpenCMISS-Dependencies/llvm
#  GIT_TAG devel

#  CMAKE_GENERATOR "Ninja"
#  UPDATE_COMMAND ""
#  PATCH_COMMAND ""
#  BUILD_COMMAND "ninja"
#  TEST_COMMAND ""

#  CMAKE_ARGS ${DEPENDENCIES_CMAKE_ARGS}
#  )

#list (APPEND DEPENDENCIES ep_clang)
#ExternalProject_Add (ep_clang
#  DEPENDS ep_llvm ep_libxml2 ep_zlib
#  GIT_REPOSITORY https://github.com/nickerso/opencmiss-clang
#  GIT_TAG backport-3.8-libxml-patch

#  UPDATE_COMMAND ""
#  PATCH_COMMAND ""
#  BUILD_COMMAND "ninja"
#  TEST_COMMAND ""

#  CMAKE_GENERATOR "Ninja"
#  CMAKE_ARGS ${DEPENDENCIES_CMAKE_ARGS}
#  )

## shouldn't need this if we can find all the config files
##list (APPEND EXTRA_CMAKE_ARGS
##  -DBOOST_ROOT=${CMAKE_CURRENT_BINARY_DIR}/Dependencies/Source/ep_boost
##  -DBoost_NO_SYSTEM_PATHS=ON)

## FIXME add to default target "all"?
#ExternalProject_Add (ep_csim
#  DEPENDS ${DEPENDENCIES}
#  SOURCE_DIR ${PROJECT_SOURCE_DIR}

#  CMAKE_GENERATOR "Ninja"
#  CMAKE_ARGS -DUSE_SUPERBUILD=OFF -DCMAKE_PREFIX_PATH=${DEPENDENCIES_INSTALL_DIR}/cmake -DBUILD_TESTING=ON

#  TEST_COMMAND "ctest"
#  INSTALL_COMMAND ""
#  BUILD_COMMAND "ninja"
#  BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/csim
#)


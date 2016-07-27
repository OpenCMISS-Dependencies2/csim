# Copyright 2015 University of Auckland
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.cmake_minimum_required (VERSION 3.1)

cmake_minimum_required(VERSION 3.2)

# Handle superbuild first
option (USE_SUPERBUILD "Whether or not a superbuild should be invoked" ON)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if (USE_SUPERBUILD)
  project (SUPERBUILD NONE)
  # execute the superbuild (this script will be invoked again without the
  # USE_SUPERBUILD option this time)
  include (cmake/SuperBuild.cmake)
  return()                      # stop processing this file further
else()
  project(CSim VERSION 1.0 LANGUAGES CXX)
endif()

option(BUILD_TESTING "Build CSim tests" OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(common)

# Use the following variables when configuring the build from the command line to
# set the corresponding cache variables.

# COVERAGE ==> CSIM_COVERAGE
set(_PARAM_ANNOTATION "Enable coverage testing.")
if(CMAKE_COMPILER_IS_GNUCXX)
  set( CSIM_COVERAGE OFF CACHE BOOL ${_PARAM_ANNOTATION} )
endif()
if( COVERAGE AND CMAKE_COMPILER_IS_GNUCXX )
  set( CSIM_COVERAGE "${COVERAGE}" CACHE BOOL ${_PARAM_ANNOTATION} FORCE )
endif()

# MEMCHECK ==> CSIM_MEMCHECK
set(_PARAM_ANNOTATION "Enable memcheck testing.")
if(CMAKE_COMPILER_IS_GNUCXX)
  set( CSIM_MEMCHECK OFF CACHE BOOL ${_PARAM_ANNOTATION} )
endif()
if( MEMCHECK AND CMAKE_COMPILER_IS_GNUCXX )
  set( CSIM_MEMCHECK "${MEMCHECK}" CACHE BOOL ${_PARAM_ANNOTATION} FORCE )
endif()

if(CSIM_COVERAGE OR CSIM_MEMCHECK)
  set(CSIM_UNIT_TESTS ON)
endif()

# TWAE ==> CSIM_WARNINGS_AS_ERRORS -- Note: This excludes third party code, where warnings are never treated as errors.
set(_PARAM_ANNOTATION "Treat warnings as errors (note: warnings in 3rd party code are never treated as errors)")
set( CSIM_TREAT_WARNINGS_AS_ERRORS OFF CACHE BOOL ${_PARAM_ANNOTATION} )
if( TWAE )
  set( CSIM_TREAT_WARNINGS_AS_ERRORS "${TWAE}" CACHE BOOL ${_PARAM_ANNOTATION} FORCE )
endif()

# BUILD_TYPE ==> CSIM_BUILD_TYPE
set(_PARAM_ANNOTATION "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel.")
set( CSIM_BUILD_TYPE "Release" CACHE STRING ${_PARAM_ANNOTATION} )
if( BUILD_TYPE )
  set( CSIM_BUILD_TYPE ${BUILD_TYPE} CACHE STRING ${_PARAM_ANNOTATION} FORCE )
endif()
if( CSIM_MEMCHECK OR CSIM_COVERAGE)
  set( CSIM_BUILD_TYPE "Debug" CACHE STRING ${_PARAM_ANNOTATION} FORCE )
endif()

# INSTALL_PREFIX ==> CSIM_INSTALL_PREFIX
set(_PARAM_ANNOTATION "Install path prefix, prepended onto install directories.")
set( CSIM_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX} CACHE PATH ${_PARAM_ANNOTATION} )
if( INSTALL_PREFIX )
  set( CSIM_INSTALL_PREFIX ${INSTALL_PREFIX} CACHE PATH ${_PARAM_ANNOTATION} FORCE )
endif()

# BUILD_SHARED ==> CSIM_BUILD_SHARED
set(_PARAM_ANNOTATION "Build shared libraries (so, dylib, DLLs).")
set( CSIM_BUILD_SHARED OFF CACHE BOOL ${_PARAM_ANNOTATION} )
if( BUILD_SHARED )
  set( CSIM_BUILD_SHARED ${BUILD_SHARED} CACHE BOOL ${_PARAM_ANNOTATION} FORCE )
endif()

# CSIM_DEPENDENCY_DIR
set(_PARAM_ANNOTATION "Dependency prefix (where to find non-system dependencies)")
set( CSIM_DEPENDENCY_DIR "/usr/local" CACHE STRING ${_PARAM_ANNOTATION} )
if( DEPENDENCY_DIR )
  set( CSIM_DEPENDENCY_DIR ${DEPENDENCY_DIR} CACHE STRING ${_PARAM_ANNOTATION} FORCE )
endif()

# Keep the GUI tidy.
internalise_cmake_variables()
hide_distracting_variables()

# cellml library target
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src)

if (BUILD_TESTING)
  # enable testing here so that we can make use of the 'test' target
  enable_testing()
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tests)
endif()

#add_subdirectory(examples/cvode-integrator)




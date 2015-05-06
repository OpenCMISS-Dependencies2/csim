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

# The version number.
set(CSIM_VERSION_MAJOR 2)
set(CSIM_VERSION_MINOR 0)
set(CSIM_VERSION_PATCH 0)

foreach(VERSION_PART CSIM_VERSION_MAJOR CSIM_VERSION_MINOR CSIM_VERSION_PATCH)
  string(LENGTH ${${VERSION_PART}} VERSION_PART_LENGTH)
  if(VERSION_PART_LENGTH EQUAL 1)
    set(${VERSION_PART}_PAD 0)
  endif()
endforeach()
set(CSIM_LIBRARY_VERSION 0x${CSIM_VERSION_MAJOR_PAD}${CSIM_VERSION_MAJOR}${CSIM_VERSION_MINOR_PAD}${CSIM_VERSION_MINOR}${CSIM_VERSION_PATCH_PAD}${CSIM_VERSION_PATCH})
set(CSIM_LIBRARY_VERSION_STRING "${CSIM_VERSION_MAJOR}.${CSIM_VERSION_MINOR}.${CSIM_VERSION_PATCH}")

set(CSIM_EXPORT_H "${CMAKE_CURRENT_BINARY_DIR}/api/csim/csim_export.h")
set(CSIM_CONFIG_H "${CMAKE_CURRENT_BINARY_DIR}/csim_config.h")
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/configure/csim_config.h.in"
  ${CSIM_CONFIG_H}
)

set(SOURCE_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/version.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/model.cpp
)

set(API_HEADER_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/api/csim/version.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/csim/model.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/csim/error_codes.h
  ${CSIM_EXPORT_H}
)

set(HEADER_FILES
  ${CSIM_CONFIG_H}
)

include(GenerateExportHeader)
add_compiler_export_flags()

add_library(csim
  ${SOURCE_FILES}
  ${HEADER_FILES}
  ${API_HEADER_FILES}
)

target_include_directories(csim
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/api>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/api>
  $<INSTALL_INTERFACE:include/>
  PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}
)

if(CSIM_TREAT_WARNINGS_AS_ERRORS)
  target_warnings_as_errors(csim)
endif()

if(CSIM_COVERAGE)
  set_target_properties(csim PROPERTIES COMPILE_FLAGS "-fprofile-arcs -ftest-coverage")
  target_link_libraries(csim gcov)

  # Share some paths with interested parties (tests)
  set(GCOV_ANALYSIS_PATH "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/csim.dir" PARENT_SCOPE)
  set(GCOV_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}" PARENT_SCOPE)
  set(GCOV_BUILD_PREFIX_PATH "${CMAKE_CURRENT_BINARY_DIR}" PARENT_SCOPE)
endif()

generate_export_header(csim EXPORT_FILE_NAME ${CSIM_EXPORT_H} BASE_NAME CSIM)

set_source_files_properties(${CSIM_EXPORT_H} PROPERTIES GENERATED TRUE)

export(TARGETS csim FILE csim-exports.cmake)

install(TARGETS csim EXPORT csim-targets
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

install(FILES
  ${API_HEADER_FILES}
  DESTINATION include/csim
)

install(EXPORT csim-targets DESTINATION lib/cmake)


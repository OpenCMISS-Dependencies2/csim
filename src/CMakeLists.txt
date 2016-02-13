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
set(CSIM_VERSION_PATCH 1)

foreach(VERSION_PART CSIM_VERSION_MAJOR CSIM_VERSION_MINOR CSIM_VERSION_PATCH)
  string(LENGTH ${${VERSION_PART}} VERSION_PART_LENGTH)
  if(VERSION_PART_LENGTH EQUAL 1)
    set(${VERSION_PART}_PAD 0)
  endif()
endforeach()
set(CSIM_LIBRARY_VERSION 0x${CSIM_VERSION_MAJOR_PAD}${CSIM_VERSION_MAJOR}${CSIM_VERSION_MINOR_PAD}${CSIM_VERSION_MINOR}${CSIM_VERSION_PATCH_PAD}${CSIM_VERSION_PATCH})
set(CSIM_LIBRARY_VERSION_STRING "${CSIM_VERSION_MAJOR}.${CSIM_VERSION_MINOR}.${CSIM_VERSION_PATCH}")

# this only works for C files
#INCLUDE (CheckIncludeFiles)
# usage: CHECK_INCLUDE_FILES (<header> <RESULT_VARIABLE> )
#CHECK_INCLUDE_FILES ("codecvt" CSIM_HAVE_STD_CODECVT)

INCLUDE (CheckCXXSourceCompiles)
check_cxx_source_compiles("
  #include <codecvt>
  int main() {
    std::codecvt_utf8_utf16<wchar_t> x;
    return 0;
  }
" CSIM_HAVE_STD_CODECVT)

set(CSIM_EXPORT_H "${CMAKE_CURRENT_BINARY_DIR}/api/csim/csim_export.h")
set(CSIM_CONFIG_H "${CMAKE_CURRENT_BINARY_DIR}/csim_config.h")
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/configure/csim_config.h.in"
  ${CSIM_CONFIG_H}
)

find_package(LibCellML ${LIBCELLML_VERSION} CONFIG REQUIRED)
find_package(LibXml2 ${LIBXML2_VERSION} REQUIRED)
find_package(ZLIB ${ZLIB_VERSION} REQUIRED)

# Since LLVM 3.5 the cmake files are available in the releases, so consolidate on the "cmake" method now.
find_package(LLVM ${LLVM_VERSION} CONFIG REQUIRED)
find_package(Clang ${CLANG_VERSION} CONFIG REQUIRED)

if( ${CMAKE_SYSTEM_NAME} STREQUAL "Linux" )
  SET(PLATFORM_LIBS ${PLATFORM_LIBS} "dl" "pthread" "tinfo")
elseif( ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin" )
  SET(PLATFORM_LIBS ${PLATFORM_LIBS} "ncurses")
endif()

set(SOURCE_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/version.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/model.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cellml_model_definition.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/compiler.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/xmlutils.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/csimsbw.cpp
)

set(API_HEADER_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/api/csim/version.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/csim/model.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/csim/error_codes.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/csim/executable_functions.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/csimsbw.h
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

target_compile_definitions(csim
  PRIVATE
  ${LLVM_DEFINITIONS}
)

target_include_directories(csim
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/api>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/api>
  $<INSTALL_INTERFACE:include/>
  PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(csim PUBLIC
  libcellml
  LLVMX86CodeGen
  LLVMExecutionEngine
  LLVMMCJIT
  clangCodeGen
  PRIVATE
  xml2
  zlib
  ${PLATFORM_LIBS}
)

set_target_properties(csim PROPERTIES
    OUTPUT_NAME csim
    VERSION ${CSIM_LIBRARY_VERSION_STRING}
)

set_target_properties(csim PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON)

if(CSIM_TREAT_WARNINGS_AS_ERRORS)
  target_warnings_as_errors(csim)
endif()

if(CSIM_COVERAGE)
  set_target_properties(csim PROPERTIES COMPILE_FLAGS "-fprofile-arcs -ftest-coverage")
  target_link_libraries(csim PRIVATE gcov)

  # Share some paths with interested parties (tests)
  set(GCOV_ANALYSIS_PATH "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/csim.dir" PARENT_SCOPE)
  set(GCOV_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}" PARENT_SCOPE)
  set(GCOV_BUILD_PREFIX_PATH "${CMAKE_CURRENT_BINARY_DIR}" PARENT_SCOPE)
endif()

generate_export_header(csim EXPORT_FILE_NAME ${CSIM_EXPORT_H} BASE_NAME CSIM)
set_source_files_properties(${CSIM_EXPORT_H} PROPERTIES GENERATED TRUE)

export(TARGETS csim FILE csim-exports.cmake)

install(TARGETS csim EXPORT csim-config
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  INCLUDES DESTINATION include
)
install(FILES
  ${API_HEADER_FILES}
  DESTINATION include/csim
)

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/csim-config-dependencies.cmake 
    "include(CMakeFindDependencyMacro)\r\n"
	"set(${PROJECT_NAME}_IMPORT_PREFIX \${_IMPORT_PREFIX})\r\n"   
    "find_dependency(LibCellML ${LIBCELLML_VERSION})\r\n"
    "find_dependency(LLVM ${LLVM_VERSION})\r\n"
    "find_dependency(ZLIB ${ZLIB_VERSION})\r\n"
    "find_dependency(LibXml2 ${LIBXML2_VERSION})\r\n"
    "find_dependency(Clang ${CLANG_VERSION})\r\n"
    "set(_IMPORT_PREFIX \${${PROJECT_NAME}_IMPORT_PREFIX})"
)

install(EXPORT csim-config DESTINATION cmake)
include(CMakePackageConfigHelpers)
WRITE_BASIC_PACKAGE_VERSION_FILE(${CMAKE_CURRENT_BINARY_DIR}/csim-config-version.cmake
    COMPATIBILITY AnyNewerVersion)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/csim-config-version.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/csim-config-dependencies.cmake
    DESTINATION cmake
)

set(CPACK_PACKAGE_VERSION_MAJOR "${CSIM_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${CSIM_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${CSIM_VERSION_PATCH}")

if(WIN32)
  set(CPACK_GENERATOR ZIP)
else()
  set(CPACK_GENERATOR TGZ)
endif()

include(CPack)

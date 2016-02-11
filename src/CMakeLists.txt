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

FIND_PACKAGE(CellML ${CELLML_VERSION} REQUIRED CONFIG)
FIND_PACKAGE(LibXml2 ${LIBXML2_VERSION} REQUIRED)

# Since LLVM 3.5 the cmake files are available in the releases, so consolidate on the "cmake" method now.
find_package(LLVM ${LLVM_VERSION} REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

llvm_map_components_to_libnames(REQ_LLVM_LIBNAMES
    instrumentation
    mcjit
    interpreter
    x86codegen
    bitwriter
    ipo
    option
    irreader
    asmparser
    linker
    runtimedyld
    executionengine
    asmprinter
    selectiondag
    objcarcopts
    x86desc
    mcparser
    codegen
    x86info
    bitreader
    scalaropts
    x86utils
    instcombine
    transformutils
    ipa
    target
    vectorize
    core
    mc
    mcdisassembler
    object
    analysis
    profiledata
    x86asmprinter
    support
)
set(REQ_LLVM_LIBRARIES)
foreach(llvmLib ${REQ_LLVM_LIBNAMES})
    find_library(foundLib_${llvmLib} ${llvmLib} PATHS ${LLVM_LIBRARY_DIRS})
    list(APPEND REQ_LLVM_LIBRARIES ${foundLib_${llvmLib}})
endforeach()

if( ${CMAKE_SYSTEM_NAME} STREQUAL "Linux" )
  SET(PLATFORM_LIBS ${PLATFORM_LIBS} "dl" "pthread" "tinfo" "z")
elseif( ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin" )
  SET(PLATFORM_LIBS ${PLATFORM_LIBS} "ncurses" "z")
endif()

set(REQ_CLANG_LIBNAMES
    clangFrontend
    clangSerialization
    clangDriver
    clangCodeGen
    clangParse
    clangSema
    clangStaticAnalyzerFrontend
    clangStaticAnalyzerCheckers
    clangStaticAnalyzerCore
    clangAnalysis
    clangRewrite
    clangAST
    clangLex
    clangBasic
    clangEdit
    clangLex
)
# Turn the clang libnames into absolute path libraries
# llvm_map_components_to_libnames seems to do this?
set(REQ_CLANG_LIBRARIES)
foreach(clangLib ${REQ_CLANG_LIBNAMES})
    find_library(foundLib_${clangLib} ${clangLib} PATHS ${LLVM_LIBRARY_DIRS})
    list(APPEND REQ_CLANG_LIBRARIES ${foundLib_${clangLib}})
endforeach()

message("LLVM defs: ${LLVM_DEFINITIONS}")
message("LLVM includes: ${LLVM_INCLUDE_DIRS}")
message("LLVM lib dir: ${LLVM_LIBRARY_DIRS}")
message("LLVM libs: ${REQ_LLVM_LIBRARIES}")
message("Clang libs: ${REQ_CLANG_LIBRARIES}")

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
  ${LLVM_INCLUDE_DIRS}
)

target_link_libraries(csim PRIVATE
  libcellml
  ${REQ_CLANG_LIBRARIES}
  ${REQ_LLVM_LIBRARIES}
  xml2
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
)

install(FILES
  ${API_HEADER_FILES}
  DESTINATION include/csim
)

install(EXPORT csim-config DESTINATION cmake)

set(CPACK_PACKAGE_VERSION_MAJOR "${CSIM_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${CSIM_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${CSIM_VERSION_PATCH}")

if(WIN32)
  set(CPACK_GENERATOR ZIP)
else()
  set(CPACK_GENERATOR TGZ)
endif()

include(CPack)

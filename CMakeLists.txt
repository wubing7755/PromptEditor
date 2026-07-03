cmake_minimum_required(VERSION 3.21)

project(PromptEditor
    VERSION 0.1.0
    DESCRIPTION "A compact C11 project template and engineering standard"
    LANGUAGES C
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(PROMPTEDITOR_IS_TOP_LEVEL_DEFAULT ON)
else()
    set(PROMPTEDITOR_IS_TOP_LEVEL_DEFAULT OFF)
endif()

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
        "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(PROMPTEDITOR_ENABLE_ASAN "Enable AddressSanitizer where supported" OFF)
option(PROMPTEDITOR_ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer where supported" OFF)
option(PROMPTEDITOR_ENABLE_COVERAGE "Enable coverage flags where supported" OFF)
option(PROMPTEDITOR_BUILD_EXAMPLE "Build the example executable" ${PROMPTEDITOR_IS_TOP_LEVEL_DEFAULT})
option(PROMPTEDITOR_BUILD_TESTING "Build CTest tests" ${PROMPTEDITOR_IS_TOP_LEVEL_DEFAULT})
option(PROMPTEDITOR_INSTALL "Generate install rules" ${PROMPTEDITOR_IS_TOP_LEVEL_DEFAULT})

set(PROMPTEDITOR_GENERATED_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated/include")
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/prompteditor_version.h.in"
    "${PROMPTEDITOR_GENERATED_INCLUDE_DIR}/prompteditor/version.h"
    @ONLY
)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/CompilerOptions.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/Dependencies.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/Sources.cmake")

add_library(prompteditor_core ${PROMPTEDITOR_CORE_SOURCES})
add_library(PromptEditor::prompteditor_core ALIAS prompteditor_core)

target_compile_features(prompteditor_core PUBLIC c_std_11)

target_compile_definitions(prompteditor_core
    PRIVATE
        PROMPTEDITOR_BUILDING_LIBRARY
)
if(BUILD_SHARED_LIBS)
    target_compile_definitions(prompteditor_core PUBLIC PROMPTEDITOR_SHARED)
endif()
set_target_properties(prompteditor_core PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

target_include_directories(prompteditor_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${PROMPTEDITOR_GENERATED_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:include>
)
prompteditor_target_defaults(prompteditor_core)

if(PROMPTEDITOR_BUILD_EXAMPLE)
    add_executable(prompteditor ${prompteditor_SOURCES})
    target_link_libraries(prompteditor PRIVATE prompteditor_core)
    prompteditor_target_defaults(prompteditor)
    set_target_properties(prompteditor PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
endif()

if(PROMPTEDITOR_BUILD_TESTING)
    enable_testing()
    include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/Tests.cmake")
endif()

if(PROMPTEDITOR_INSTALL)
    include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/Packaging.cmake")
endif()

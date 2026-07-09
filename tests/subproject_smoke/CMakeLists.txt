cmake_minimum_required(VERSION 3.21)

project(PromptLibSubprojectSmoke LANGUAGES C)

if(NOT DEFINED PP_SOURCE_DIR)
    message(FATAL_ERROR "PP_SOURCE_DIR must point at the template source tree")
endif()

add_subdirectory("${PP_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/pp-standard")

if(NOT TARGET PromptLib::promptlib_core)
    message(FATAL_ERROR "Expected PromptLib::promptlib_core alias target")
endif()

foreach(option_name IN ITEMS
        PP_BUILD_EXAMPLE
        PP_BUILD_TESTING
        PP_INSTALL)
    if(${option_name})
        message(FATAL_ERROR "${option_name} should default to OFF for subproject builds")
    endif()
endforeach()

foreach(target_name IN ITEMS
        pp
        pp_tests
        pp_tests)
    if(TARGET ${target_name})
        message(FATAL_ERROR "${target_name} should not be created by default for subproject builds")
    endif()
endforeach()

add_executable(subproject_smoke main.c)
target_link_libraries(subproject_smoke PRIVATE PromptLib::promptlib_core)

enable_testing()
add_test(NAME subproject_smoke COMMAND subproject_smoke)

cmake_minimum_required(VERSION 3.21)

project(PromptEditorSubprojectSmoke LANGUAGES C)

if(NOT DEFINED PROMPTEDITOR_SOURCE_DIR)
    message(FATAL_ERROR "PROMPTEDITOR_SOURCE_DIR must point at the template source tree")
endif()

add_subdirectory("${PROMPTEDITOR_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/prompteditor-standard")

if(NOT TARGET PromptEditor::prompteditor_core)
    message(FATAL_ERROR "Expected PromptEditor::prompteditor_core alias target")
endif()

foreach(option_name IN ITEMS
        PROMPTEDITOR_BUILD_EXAMPLE
        PROMPTEDITOR_BUILD_TESTING
        PROMPTEDITOR_INSTALL)
    if(${option_name})
        message(FATAL_ERROR "${option_name} should default to OFF for subproject builds")
    endif()
endforeach()

foreach(target_name IN ITEMS
        prompteditor
        prompteditor_tests
        prompteditor_tests)
    if(TARGET ${target_name})
        message(FATAL_ERROR "${target_name} should not be created by default for subproject builds")
    endif()
endforeach()

add_executable(subproject_smoke main.c)
target_link_libraries(subproject_smoke PRIVATE PromptEditor::prompteditor_core)

enable_testing()
add_test(NAME subproject_smoke COMMAND subproject_smoke)

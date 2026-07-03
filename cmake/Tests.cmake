function(prompteditor_add_test target)
    cmake_parse_arguments(PROMPTEDITOR_TEST
        ""
        ""
        "SOURCES;LIBS"
        ${ARGN}
    )

    if(NOT PROMPTEDITOR_TEST_SOURCES)
        message(FATAL_ERROR "prompteditor_add_test(${target}) requires SOURCES")
    endif()

    add_executable(${target}
        ${PROMPTEDITOR_TEST_SOURCES}
    )

    target_include_directories(${target}
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/support
    )

    if(PROMPTEDITOR_TEST_LIBS)
        target_link_libraries(${target} PRIVATE ${PROMPTEDITOR_TEST_LIBS})
    endif()

    prompteditor_target_defaults(${target})

    set_target_properties(${target} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )

    add_test(NAME ${target} COMMAND $<TARGET_FILE:${target}>)
    list(APPEND PROMPTEDITOR_TEST_TARGETS ${target})
    set(PROMPTEDITOR_TEST_TARGETS "${PROMPTEDITOR_TEST_TARGETS}" PARENT_SCOPE)
endfunction()

set(PROMPTEDITOR_TEST_TARGETS "")

prompteditor_add_test(prompteditor_tests
    SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_example.c
    LIBS
        prompteditor_core
)

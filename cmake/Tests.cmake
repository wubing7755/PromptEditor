function(pp_add_test target)
    cmake_parse_arguments(PP_TEST
        ""
        ""
        "SOURCES;LIBS"
        ${ARGN}
    )

    if(NOT PP_TEST_SOURCES)
        message(FATAL_ERROR "pp_add_test(${target}) requires SOURCES")
    endif()

    add_executable(${target}
        ${PP_TEST_SOURCES}
    )

    target_include_directories(${target}
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/support
    )

    if(PP_TEST_LIBS)
        target_link_libraries(${target} PRIVATE ${PP_TEST_LIBS})
    endif()

    pp_target_defaults(${target})

    set_target_properties(${target} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )

    add_test(NAME ${target} COMMAND $<TARGET_FILE:${target}>)
    list(APPEND PP_TEST_TARGETS ${target})
    set(PP_TEST_TARGETS "${PP_TEST_TARGETS}" PARENT_SCOPE)
endfunction()

set(PP_TEST_TARGETS "")

pp_add_test(pp_tests
    SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_example.c
    LIBS
        promptlib_core
)

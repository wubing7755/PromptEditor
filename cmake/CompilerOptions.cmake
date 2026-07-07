function(pp_target_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive-)
        target_compile_definitions(${target} PRIVATE _CRT_SECURE_NO_WARNINGS)
    else()
        target_compile_options(${target} PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Wshadow
            -Wconversion
        )
    endif()
endfunction()

function(pp_target_sanitizers target)
    if(PP_ENABLE_ASAN)
        if(MSVC)
            target_compile_options(${target} PRIVATE /fsanitize=address)
            target_link_options(${target} PRIVATE /fsanitize=address)
        else()
            target_compile_options(${target} PRIVATE -fsanitize=address)
            target_link_options(${target} PRIVATE -fsanitize=address)
        endif()
    endif()

    if(PP_ENABLE_UBSAN)
        if(NOT MSVC)
            target_compile_options(${target} PRIVATE -fsanitize=undefined)
            target_link_options(${target} PRIVATE -fsanitize=undefined)
        else()
            message(WARNING
                "PP_ENABLE_UBSAN is ON, but UBSAN is not configured for MSVC in this template.")
        endif()
    endif()
endfunction()

function(pp_target_coverage target)
    if(NOT PP_ENABLE_COVERAGE)
        return()
    endif()

    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
        target_compile_options(${target} PRIVATE --coverage -O0 -g)
        target_link_options(${target} PRIVATE --coverage)
    else()
        message(WARNING
            "PP_ENABLE_COVERAGE is ON, but coverage flags are only configured for GCC and Clang-like compilers.")
    endif()
endfunction()

function(pp_target_defaults target)
    target_compile_features(${target} PRIVATE c_std_11)
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        target_compile_definitions(${target} PRIVATE _GNU_SOURCE)
    endif()
    pp_target_warnings(${target})
    pp_target_sanitizers(${target})
    pp_target_coverage(${target})
endfunction()

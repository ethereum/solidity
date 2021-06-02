# Cable: CMake Bootstrap Library <https://github.com/ethereum/cable>
# Copyright 2021 Pawel Bylica.
# Licensed under the Apache License, Version 2.0.

# Cable Compile Options, version 1.0.0
#
# This CMake module provides utilities to conditionally add compile options
# depending on language and compiler support.
#
# CHANGELOG
#
# 1.0.0 - 2021-03-24

include_guard(GLOBAL)

include(CheckCXXCompilerFlag)
include(CheckCCompilerFlag)

function(cable_add_compile_options)
    cmake_parse_arguments(ARG "" "" "IF_SUPPORTED" ${ARGN})

    # Init options list with all arguments before IF_SUPPORTED keyword.
    set(options ${ARG_UNPARSED_ARGUMENTS})

    # Get list of languages to check.
    # Currently only C and CXX is supported here, but with check_compiler_flag()
    # from CMake 3.19 all languages can be checked.
    get_property(languages GLOBAL PROPERTY ENABLED_LANGUAGES)
    list(FILTER languages INCLUDE REGEX "C|CXX")

    # Some flags causes compiler warning instead of error. Still such flags must
    # be considered unsupported. To make check_X_compiler_flag() fail before
    # CMake 3.19 additional -Werror flag must be added.
    list(GET languages 0 example_lang)
    set(compiler ${CMAKE_${example_lang}_COMPILER_ID})
    if(compiler MATCHES GNU OR compiler MATCHES Clang)
        set(CMAKE_REQUIRED_FLAGS -Werror)
    endif()

    foreach(flag ${ARG_IF_SUPPORTED})
        string(MAKE_C_IDENTIFIER ${flag} flag_id)
        set(supported_in_all_languages TRUE)

        foreach(lang ${languages})
            set(result_var "${lang}${flag_id}")

            # Check if the flag works in the lang's compiler.
            # In CMake 3.18+ cmake_language(CALL ...) can be used.
            if(lang STREQUAL CXX)
                check_cxx_compiler_flag(${flag} ${result_var})
            else()
                check_c_compiler_flag(${flag} ${result_var})
            endif()

            if(NOT "${${result_var}}")
                set(supported_in_all_languages FALSE)
            endif()
        endforeach()

        if(supported_in_all_languages)
            list(APPEND options ${flag})
        else()
            foreach(lang ${languages})
                set(result_var "${lang}${flag_id}")
                if(${${result_var}})
                    list(APPEND options "$<$<COMPILE_LANGUAGE:${lang}>:${flag}>")
                endif()
            endforeach()
        endif()
    endforeach()

    add_compile_options(${options})
endfunction()

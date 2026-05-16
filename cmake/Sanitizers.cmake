# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

include( CheckCXXCompilerFlag )

macro( add_sanitizer name )
    set( CMAKE_REQUIRED_FLAGS "-fsanitize=${name}" )
    check_cxx_compiler_flag( -fsanitize=${name} COMPILER_SUPPORTS_SANITIZE_${name} )
    if( COMPILER_SUPPORTS_SANITIZE_${name} )
        message( STATUS "Supported sanitizer: ${name}" )
        list( APPEND BIT7Z_SANITIZER_FLAGS "-fsanitize=${name}" )
    else()
        message( STATUS "Unsupported sanitizer: ${name}" )
    endif()
    unset( CMAKE_REQUIRED_FLAGS )
endmacro()

macro( remove_sanitizer name )
    set( CMAKE_REQUIRED_FLAGS "-fno-sanitize=${name}" )
    check_cxx_compiler_flag( -fno-sanitize=${name} COMPILER_SUPPORTS_NO_SANITIZE_${name} )
    if( COMPILER_SUPPORTS_NO_SANITIZE_${name} )
        message( STATUS "Removed sanitizer: ${name}" )
        list( APPEND BIT7Z_SANITIZER_FLAGS "-fno-sanitize=${name}" )
    else()
        message( STATUS "Could not remove sanitizer: ${name}" )
    endif()
    unset( CMAKE_REQUIRED_FLAGS )
endmacro()

option( BIT7Z_ENABLE_SANITIZERS "Enable or disable compiling with sanitizers" )
message( STATUS "Enable sanitizers: ${BIT7Z_ENABLE_SANITIZERS}" )
if( BIT7Z_ENABLE_SANITIZERS )
    if( MSVC )
        target_compile_options( ${LIB_TARGET} PUBLIC
            $<$<CONFIG:Debug>:/RTCsu>
            $<$<CONFIG:Debug>:/analyze>
            $<$<CONFIG:Debug>:/guard:cf> )
    elseif( NOT WIN32 ) # GCC/Clang on Linux/macOS (i.e., not MinGW)
        target_compile_options( ${LIB_TARGET} PUBLIC $<$<CONFIG:Debug>:-fno-omit-frame-pointer> )

        if( CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "6.0.0" AND NOT APPLE )
            target_link_options( ${LIB_TARGET} PUBLIC $<$<CONFIG:Debug>:-static-libsan> )
        endif()

        # For some reasons, the address sanitizer gives a CHECK failed error on versions of Clang before the 3.9
        # Also, on old versions of GCC the sanitizer give some false positives.
        if( ( NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "5.0" ) OR
            ( CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "3.9.0" ) )
            add_sanitizer( address )
            if( COMPILER_SUPPORTS_SANITIZE_address )
                check_cxx_compiler_flag( -fsanitize-address-use-after-scope COMPILER_SUPPORT_SANITIZE_USE_AFTER_SCOPE )
                if( COMPILER_SUPPORT_SANITIZE_USE_AFTER_SCOPE )
                    message( STATUS "Supported sanitizer: address-use-after-scope" )
                    target_compile_options( ${LIB_TARGET} PUBLIC $<$<CONFIG:Debug>:-fsanitize-address-use-after-scope> )
                else()
                    message( STATUS "Unsupported sanitizer: address-use-after-scope" )
                endif()
            endif()
        endif()

        add_sanitizer( float-divide-by-zero )
        add_sanitizer( implicit-conversion )
        if( CMAKE_CXX_COMPILER_ID MATCHES "Clang" )
            add_sanitizer( integer )
            remove_sanitizer( unsigned-shift-base )
            remove_sanitizer( unsigned-integer-overflow )
        endif()
        add_sanitizer( leak )
        add_sanitizer( local-bounds )
        add_sanitizer( nullability )
        add_sanitizer( undefined )

        foreach( _flag IN LISTS BIT7Z_SANITIZER_FLAGS )
            target_compile_options( ${LIB_TARGET} PUBLIC $<$<CONFIG:Debug>:${_flag}> )
            target_link_options( ${LIB_TARGET} PUBLIC $<$<CONFIG:Debug>:${_flag}> )
        endforeach()

        check_cxx_compiler_flag( -fstack-protector-strong COMPILER_SUPPORT_STRONG_STACK_PROTECTOR )
        check_cxx_compiler_flag( -fstack-protector COMPILER_SUPPORT_STACK_PROTECTOR )
        if( COMPILER_SUPPORT_STRONG_STACK_PROTECTOR )
            target_compile_options( ${LIB_TARGET} PUBLIC -fstack-protector-strong )
        elseif( COMPILER_SUPPORT_STACK_PROTECTOR )
            target_compile_options( ${LIB_TARGET} PUBLIC -fstack-protector )
        endif()
    endif()
endif()

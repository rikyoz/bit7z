# compiler-specific options

if( MSVC )
    # setting a pdb file name for debug builds (otherwise it is not generated!)
    set_target_properties( ${TARGET_NAME} PROPERTIES COMPILE_PDB_NAME_DEBUG ${TARGET_NAME}${CMAKE_DEBUG_POSTFIX} )

    # release builds should be optimized (e.g., for size)
    target_compile_options( ${TARGET_NAME} PRIVATE "$<$<OR:$<CONFIG:RELEASE>,$<CONFIG:MINSIZEREL>>:/Os>" )
    target_compile_options( ${TARGET_NAME} PRIVATE "$<$<OR:$<CONFIG:RELEASE>,$<CONFIG:MINSIZEREL>>:/Oi>" )
    target_compile_options( ${TARGET_NAME} PRIVATE "$<$<OR:$<CONFIG:RELEASE>,$<CONFIG:MINSIZEREL>>:/GS>" )
    target_compile_options( ${TARGET_NAME} PRIVATE "$<$<OR:$<CONFIG:RELEASE>,$<CONFIG:MINSIZEREL>>:/Gy>" )

    option( BIT7Z_ANALYZE_CODE "Enable or disable code analysis" )
    message( STATUS "Code analysis: ${BIT7Z_ANALYZE_CODE}" )
    if( BIT7Z_ANALYZE_CODE )
        target_compile_options( ${TARGET_NAME} PRIVATE "$<$<OR:$<CONFIG:RELEASE>,$<CONFIG:MINSIZEREL>>:/analyze>" )
    endif()

    # remove CMake default warning level
    string( REGEX REPLACE "/W[0-4]" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" )
    string( REGEX REPLACE "/GR" "/GR-" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" )

    # warning flags (as suggested in https://lefticus.gitbooks.io/cpp-best-practices/)
    target_compile_options( ${TARGET_NAME} PRIVATE /W4 /w14640 /w14242 /w14254 /w14263 /w14265 /w14287 /we4289 /w14296
                            /w14311 /w14545 /w14546 /w14547 /w14549 /w14555 /w14619 /w14640 /w14826 /w14905 /w14906
                            /w14928 )

    # C++ standard conformance options of MSVC
    target_compile_options( ${TARGET_NAME} PRIVATE /fp:precise /Zc:wchar_t /Zc:rvalueCast /Zc:inline
                            /Zc:forScope /Zc:strictStrings /Zc:throwingNew /Zc:referenceBinding )

    if( CMAKE_GENERATOR MATCHES "Visual Studio" )
        target_compile_options( ${TARGET_NAME} PRIVATE /MP$ENV{NUMBER_OF_PROCESSORS} )
    endif()

    # linker flags
    set( CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} /nologo" )

    # options only for specific MSVC versions
    if( MSVC_VERSION GREATER_EQUAL 1910 ) # MSVC >= 15.0 (VS 2017)
        target_compile_options( ${TARGET_NAME} PRIVATE /Zc:__cplusplus )
        # treating warnings as errors
        target_compile_options( ${TARGET_NAME} PRIVATE /WX )
    else() # MSVC < 15.0 (i.e., <= VS 2015)
        # ignoring C4127 warning
        target_compile_options( ${TARGET_NAME} PRIVATE /wd4127 )
    endif()

    # static runtime option
    option( BIT7Z_STATIC_RUNTIME "Enable or disable using /MT MSVC flag" )
    message( STATUS "Static runtime: ${BIT7Z_STATIC_RUNTIME}" )
    if( BIT7Z_STATIC_RUNTIME )
        set( CompilerFlags
             CMAKE_CXX_FLAGS_DEBUG
             CMAKE_CXX_FLAGS_RELEASE
             CMAKE_C_FLAGS_DEBUG
             CMAKE_C_FLAGS_RELEASE )
        foreach( CompilerFlag ${CompilerFlags} )
            string( REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}" )
            set( ${CompilerFlag} "${${CompilerFlag}}" CACHE STRING "msvc compiler flags" FORCE )
            message( STATUS "MSVC flags: ${CompilerFlag}:${${CompilerFlag}}" )
        endforeach()
    endif()
elseif( MINGW )
    target_compile_options( ${TARGET_NAME} PRIVATE -Wall )
else()
    target_compile_options( ${TARGET_NAME} PRIVATE -Wall -Wextra -Werror )
endif()

if( CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 3.6 )
    target_compile_options( ${TARGET_NAME} PRIVATE -Wno-inconsistent-missing-override )
endif()

if( APPLE )
    set( CMAKE_CXX_ARCHIVE_CREATE "<CMAKE_AR> Scr <TARGET> <LINK_FLAGS> <OBJECTS>" )
    set( CMAKE_CXX_ARCHIVE_FINISH "<CMAKE_RANLIB> -no_warning_for_no_symbols -c <TARGET>" )
endif()

if( CMAKE_CXX_COMPILER_ID MATCHES "GNU" )
    if( CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.0 )
        target_compile_options( ${TARGET_NAME} PRIVATE -Wno-missing-field-initializers )
    endif()
    if( CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 6.0 )
        # GCC 6.0+ complains on 7-zip macros using misleading indentation, disabling the warning to make it compile.
        target_compile_options( ${TARGET_NAME} PRIVATE -Wno-misleading-indentation )
    endif()
endif()
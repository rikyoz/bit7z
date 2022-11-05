# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

# bit7z build options

option( BIT7Z_AUTO_FORMAT "Enable or disable auto format detection" )
message( STATUS "Auto format detection: ${BIT7Z_AUTO_FORMAT}" )
if( BIT7Z_AUTO_FORMAT )
    target_compile_definitions( ${LIB_TARGET} PUBLIC BIT7Z_AUTO_FORMAT )
endif()

option( BIT7Z_REGEX_MATCHING "Enable or disable regex matching of archived files" )
message( STATUS "Regex matching extraction: ${BIT7Z_REGEX_MATCHING}" )
if( BIT7Z_REGEX_MATCHING )
    target_compile_definitions( ${LIB_TARGET} PUBLIC BIT7Z_REGEX_MATCHING )
endif()

option( BIT7Z_USE_STD_BYTE "Enable or disable using type safe byte type (like std::byte) for buffers" )
message( STATUS "Use std::byte: ${BIT7Z_USE_STD_BYTE}" )
if( BIT7Z_USE_STD_BYTE )
    target_compile_definitions( ${LIB_TARGET} PUBLIC BIT7Z_USE_STD_BYTE )
endif()

option( BIT7Z_USE_NATIVE_STRING "Enable or disable using the OS native string type
                                 (e.g., std::wstring on Windows, std::string elsewhere)" )
message( STATUS "Use native string: ${BIT7Z_USE_NATIVE_STRING}" )
if( BIT7Z_USE_NATIVE_STRING )
    target_compile_definitions( ${LIB_TARGET} PUBLIC BIT7Z_USE_NATIVE_STRING )
endif()

option( BIT7Z_BUILD_TESTS "Enable or disable building the testing executable" )
message( STATUS "Build tests: ${BIT7Z_BUILD_TESTS}" )
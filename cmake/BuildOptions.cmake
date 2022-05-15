# bit7z build options

option( BIT7Z_AUTO_FORMAT "Enable or disable auto format detection" )
message( STATUS "Auto format detection: ${BIT7Z_AUTO_FORMAT}" )
if( BIT7Z_AUTO_FORMAT )
    target_compile_definitions( ${TARGET_NAME} PUBLIC BIT7Z_AUTO_FORMAT )
endif()

option( BIT7Z_REGEX_MATCHING "Enable or disable regex matching of archived files" )
message( STATUS "Regex matching extraction: ${BIT7Z_REGEX_MATCHING}" )
if( BIT7Z_REGEX_MATCHING )
    target_compile_definitions( ${TARGET_NAME} PUBLIC BIT7Z_REGEX_MATCHING )
endif()

option( BIT7Z_USE_STD_BYTE "Enable or disable using type safe byte type (like std::byte) for buffers" )
message( STATUS "Use std::byte: ${BIT7Z_USE_STD_BYTE}" )
if( BIT7Z_USE_STD_BYTE )
    target_compile_definitions( ${TARGET_NAME} PUBLIC BIT7Z_USE_STD_BYTE )
endif()

option( BIT7Z_USE_NATIVE_STRING "Enable or disable using the OS native string type
                                 (e.g., std::wstring on Windows, std::string elsewhere)" )
message( STATUS "Use native string: ${BIT7Z_USE_NATIVE_STRING}" )
if( BIT7Z_USE_NATIVE_STRING )
    target_compile_definitions( ${TARGET_NAME} PUBLIC BIT7Z_USE_NATIVE_STRING )
endif()
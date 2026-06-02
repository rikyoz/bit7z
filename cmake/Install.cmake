# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

# Install rules and CMake package configuration files.

option( BIT7Z_INSTALL "Generate the install target and the CMake package configuration files" ON )

if( BIT7Z_INSTALL )
    include( GNUInstallDirs )
    include( CMakePackageConfigHelpers )

    # The public headers are installed via the PUBLIC_HEADER target property (all of them live in a
    # single directory, so they are flattened into ${CMAKE_INSTALL_INCLUDEDIR}/bit7z).
    set_target_properties( ${LIB_TARGET} PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}" )

    # Targets to install and export: the canonical "bit7z" target plus, on 64-bit builds, the
    # backward-compatible "bit7z64" interface target.
    set( BIT7Z_EXPORT_TARGETS ${LIB_TARGET} )
    if( BIT7Z_TARGET_ARCH_IS_64_BIT )
        list( APPEND BIT7Z_EXPORT_TARGETS bit7z64 )
    endif()

    install( TARGETS ${BIT7Z_EXPORT_TARGETS}
             EXPORT bit7zTargets
             RUNTIME       DESTINATION ${CMAKE_INSTALL_BINDIR}
             LIBRARY       DESTINATION ${CMAKE_INSTALL_LIBDIR}
             ARCHIVE       DESTINATION ${CMAKE_INSTALL_LIBDIR}
             PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/bit7z )

    install( EXPORT bit7zTargets
             NAMESPACE bit7z::
             DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/bit7z )

    configure_package_config_file(
        "${PROJECT_SOURCE_DIR}/cmake/bit7zConfig.cmake.in"
        "${PROJECT_BINARY_DIR}/bit7zConfig.cmake"
        INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/bit7z" )

    write_basic_package_version_file(
        "${PROJECT_BINARY_DIR}/bit7zConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion )

    install( FILES "${PROJECT_BINARY_DIR}/bit7zConfig.cmake"
                   "${PROJECT_BINARY_DIR}/bit7zConfigVersion.cmake"
             DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/bit7z" )
endif()
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

find_package( Doxygen REQUIRED dot )

if( DOXYGEN_FOUND )
    if(POLICY CMP0135)
        cmake_policy(SET CMP0135 NEW)
    endif()

    set( EXTRA_PATH ${CMAKE_CURRENT_BINARY_DIR}/extra )
    set( DOXYGEN_AWESOME_PATH ${CMAKE_CURRENT_BINARY_DIR}/doxygen-awesome-css )
    set( CPPREFERENCE_PATH ${CMAKE_CURRENT_BINARY_DIR}/cppreference )

    # Dependencies needed to build the documentation
    set(FETCHCONTENT_QUIET FALSE)
    FetchContent_Declare(
            doxygen-extra
            GIT_REPOSITORY https://gist.github.com/db15772ef4daa2259c2aaff11c64441a.git
            GIT_TAG 0ee904a82c7fab446934200ad53667621ad9082a
            GIT_PROGRESS ON
            GIT_SHALLOW ON
            SOURCE_DIR ${EXTRA_PATH}
    )
    FetchContent_Declare(
            doxygen-awesome
            GIT_REPOSITORY https://github.com/jothepro/doxygen-awesome-css
            GIT_TAG main
            GIT_PROGRESS ON
            GIT_SHALLOW ON
            SOURCE_DIR ${DOXYGEN_AWESOME_PATH}
    )
    FetchContent_Declare(
            doxygen-cppreference
            URL https://upload.cppreference.com/mwiki/images/f/f8/cppreference-doxygen-web.tag.xml
            DOWNLOAD_NO_EXTRACT ON
            DOWNLOAD_DIR ${CPPREFERENCE_PATH}
    )
    FetchContent_MakeAvailable( doxygen-extra doxygen-awesome doxygen-cppreference )

    file(READ ${CMAKE_SOURCE_DIR}/README.md HOME_MD)
    string(REGEX REPLACE "<h1 align=\"center\">([^<]+)<\\/h1>" "# \\1" HOME_MD "${HOME_MD}")
    string(REGEX REPLACE "\n<!-- navbar -->(.+)<!-- navbar -->\n" "" HOME_MD "${HOME_MD}")
    file(WRITE  ${CMAKE_CURRENT_BINARY_DIR}/Home.md "${HOME_MD}")

    # set input and output files
    set( DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in )
    set( DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile )
    configure_file( ${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY )

    add_custom_target( bit7z-docs ALL
                       COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
                       WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                       COMMENT "Generating API documentation with Doxygen"
                       DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Home.md
                       VERBATIM )
else()
    message( "Doxygen need to be installed to generate the doxygen documentation" )
endif()
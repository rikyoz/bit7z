# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

# download catch2 unit testing framework

include( FetchContent )

FetchContent_declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v2.13.10
)

FetchContent_MakeAvailable( Catch2 )

set_property( GLOBAL PROPERTY CTEST_TARGETS_ADDED 1 )

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/contrib/)
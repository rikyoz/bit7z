/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2023 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef BITSHAREDLIBRARY_HPP
#define BITSHAREDLIBRARY_HPP

#include "bitdefines.hpp"
#include "bittypes.hpp"
#include "bitwindows.hpp"

namespace bit7z {

#ifdef _WIN32
using LibraryHandle = HMODULE;
using LibrarySymbol = FARPROC;
#else
using LibraryHandle = void*;
using LibrarySymbol = void*;
#endif



template< typename T >
struct is_function_pointer {
    static const bool value =
        std::is_pointer< T >::value && std::is_function< typename std::remove_pointer< T >::type >::value;
};

class BitSharedLibrary {
    public:
        explicit BitSharedLibrary( const tstring& libraryPath );

        BitSharedLibrary( const BitSharedLibrary& ) = delete;

        BitSharedLibrary( BitSharedLibrary&& ) = delete;

        auto operator=( const BitSharedLibrary& ) -> BitSharedLibrary& = delete;

        auto operator=( BitSharedLibrary&& ) -> BitSharedLibrary& = delete;

        ~BitSharedLibrary();

        BIT7Z_NODISCARD
        auto getSymbol( const char* symbolName ) -> LibrarySymbol;

        template< typename Function,
                  typename = typename std::enable_if< is_function_pointer< Function >::value >::type >
        BIT7Z_NODISCARD
        auto getFunction( const char* symbolName ) -> Function {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
            return reinterpret_cast< Function >( getSymbol( symbolName ) );
        }

    private:
        LibraryHandle mLibrary;
};

} // namespace bit7z

#endif //BITSHAREDLIBRARY_HPP

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2026 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef BIT7ZLIBRARYLOADER_HPP
#define BIT7ZLIBRARYLOADER_HPP

#include "bit7zlibrary.hpp"
#include "bitdefines.hpp"
#include "bittypes.hpp"

#include <system_error>

namespace bit7z {

/**
 * @brief The Bit7zLibraryLoader class provides deferred and optional loading of the 7-zip shared library.
 *
 * Unlike Bit7zLibrary, which loads the shared library in its constructor,
 * Bit7zLibraryLoader allows constructing an empty loader and loading the library at a later time.
 * It also supports unloading and reloading the library, as well as non-throwing error handling.
 *
 * Once loaded, the underlying Bit7zLibrary can be accessed via the library() method,
 * the arrow operator, or implicit conversion.
 */
class Bit7zLibraryLoader final {
    public:
        /**
         * @brief Constructs an empty Bit7zLibraryLoader without loading any library.
         */
        Bit7zLibraryLoader() noexcept;

        /**
         * @brief Constructs a Bit7zLibraryLoader and immediately loads the specified 7-zip shared library.
         *
         * @param libraryPath  the path to the 7-zip shared library to be loaded.
         */
        explicit Bit7zLibraryLoader( const tstring& libraryPath );

        Bit7zLibraryLoader( const Bit7zLibraryLoader& ) = delete;

        Bit7zLibraryLoader( Bit7zLibraryLoader&& ) = delete;

        auto operator=( const Bit7zLibraryLoader& ) -> Bit7zLibraryLoader& = delete;

        auto operator=( Bit7zLibraryLoader&& ) -> Bit7zLibraryLoader& = delete;

        /**
         * @brief Destructs the Bit7zLibraryLoader, unloading the library if it was loaded.
         */
        ~Bit7zLibraryLoader();

        /**
         * @brief Loads the specified 7-zip shared library.
         *
         * If a library is already loaded, it is unloaded first.
         *
         * @note A failed load leaves the loader in an unloaded state even if it was previously loaded.
         *
         * @param libraryPath  the path to the 7-zip shared library to be loaded.
         */
        void load( const tstring& libraryPath );

        /**
         * @brief Loads the specified 7-zip shared library (non-throwing overload).
         *
         * If a library is already loaded, it is unloaded first.
         *
         * @note A failed load leaves the loader in an unloaded state even if it was previously loaded.
         *
         * @param libraryPath  the path to the 7-zip shared library to be loaded.
         * @param ec           an output error code indicating the result of the operation.
         */
        void load( const tstring& libraryPath, std::error_code& ec ) noexcept;

        /**
         * @brief Returns a reference to the loaded Bit7zLibrary.
         *
         * @throws BitException if no library is currently loaded.
         *
         * @return a const reference to the underlying Bit7zLibrary.
         */
        BIT7Z_NODISCARD
        auto library() const -> const Bit7zLibrary&;

        /**
         * @brief Unloads the currently loaded library, if any.
         */
        void unload() noexcept;

        /**
         * @brief Checks whether a 7-zip shared library is currently loaded.
         *
         * @return true if a library is loaded, false otherwise.
         */
        BIT7Z_NODISCARD
        auto isLoaded() const noexcept -> bool;

        /**
         * @brief Provides pointer-like access to the loaded Bit7zLibrary.
         *
         * @throws BitException if no library is currently loaded.
         *
         * @return a pointer to the underlying Bit7zLibrary.
         */
        BIT7Z_NODISCARD
        auto operator->() const -> const Bit7zLibrary*;

        /**
         * @brief Implicitly converts to the loaded Bit7zLibrary reference.
         *
         * This allows passing a Bit7zLibraryLoader directly where a Bit7zLibrary reference is expected.
         *
         * @throws BitException if no library is currently loaded.
         */
        /* implicit */ operator const Bit7zLibrary&() const; //NOLINT(*-explicit-conversions)

    private:
        /* We can't use std::optional here since bit7z's public API is still C++14.
         * Alternatively, we could move the internal's Optional class to the public API, and use it here.
         * Ultimately, I preferred to not pollute the public API with yet another class and
         * simply reimplemented a very bare-bones optional storage. */
        union {
            byte_t mEmpty;
            Bit7zLibrary mLibrary;
        };
        bool mLoaded;
};

} // namespace bit7z

#endif //BIT7ZLIBRARYLOADER_HPP

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef BITABSTRACTARCHIVEHANDLER_HPP
#define BITABSTRACTARCHIVEHANDLER_HPP

#include "bit7zlibrary.hpp"
#include "bitdefines.hpp"
#include "bittypes.hpp"

#include <cstdint>
#include <functional>

namespace bit7z {

class BitInFormat;
class BitArchiveItem;

/**
 * @brief A function whose argument is the total size of the ongoing operation.
 */
using TotalCallback = std::function< void( std::uint64_t ) >;

/**
 * @brief A function whose argument is the currently processed size of the ongoing operation and returns
 *        true or false whether the operation must continue or not.
 */
using ProgressCallback = std::function< bool( std::uint64_t ) >;

/**
 * @brief A function whose arguments are the current processed input size, and the current output size of the
 *        ongoing operation.
 */
using RatioCallback = std::function< void( std::uint64_t, std::uint64_t ) >;

/**
 * @brief A function whose argument is the path, in the archive, of the file currently being processed
 *        by the ongoing operation.
 */
using FileCallback = std::function< void( const tstring& ) >;

/**
 * @brief A function returning the password to be used to handle an archive.
 */
using PasswordCallback = std::function< tstring() >;

/**
 * @brief A function providing the raw extracted data and its size to the user.
 */
using RawDataCallback = std::function< bool( const byte_t*, std::size_t ) >;

/**
 * @brief A function returning a new name for the item currently being extracted.
 *
 * @note The callback receives the archive item being extracted and must return the path that
 * the extracted item must have on the filesystem. Returning the item's own path leaves it
 * unchanged; returning an empty string skips the item.
 */
using RenameCallback = std::function< tstring( const BitArchiveItem& ) >;

/**
 * @brief The (index, path) form of RenameCallback.
 *
 * @deprecated Since v4.1; it will be removed in v4.2. Use RenameCallback, which receives the
 * BitArchiveItem being extracted (its index and path are available via the item).
 */
using LegacyRenameCallback = std::function< tstring( std::uint32_t, const tstring& ) >;

/**
 * @brief A function returning a reference to the buffer where to extract the item at the given index/path.
 */
using BufferCallback = std::function< buffer_t&( std::uint32_t, const tstring& ) >;

/**
 * @brief Enumeration representing the result of applying a filter callback on an archive item.
 */
enum struct FilterResult : std::uint8_t {
    ProcessItem,    ///< Continue processing the item.
    SkipItem,       ///< Skip the item (do not process it).
    AbortOperation, ///< Abort the whole operation.
    BIT7Z_DEPRECATED_ENUMERATOR( Process, ProcessItem, "Use FilterResult::ProcessItem" ),
    BIT7Z_DEPRECATED_ENUMERATOR( Skip, SkipItem, "Use FilterResult::SkipItem" ),
    BIT7Z_DEPRECATED_ENUMERATOR( Abort, AbortOperation, "Use FilterResult::AbortOperation" ),
};

/**
 * @brief A function returning what to do with the given archive item.
 */
using FilterCallback = std::function< FilterResult( const BitArchiveItem& ) >;

/**
 * @brief Enumeration representing how a handler should deal when an output file already exists.
 */
enum struct OverwriteMode : std::uint8_t {
    None = 0,  ///< The handler will throw an exception if the output file or buffer already exists.
    Overwrite, ///< The handler will overwrite the old file or buffer with the new one.
    Skip,      ///< The handler will skip writing to the output file or buffer.
    //TODO:    RenameOutput,
    //TODO:    RenameExisting
};

/**
 * @brief Enumeration representing the policy according to which the archive handler should treat
 *        the items that match the pattern given by the user.
 */
enum struct FilterPolicy : std::uint8_t {
    Include, ///< Extract/compress the items that match the pattern.
    Exclude  ///< Do not extract/compress the items that match the pattern.
};

/**
 * @brief Abstract class representing a generic archive handler.
 */
class BitAbstractArchiveHandler {
    public:
        BitAbstractArchiveHandler( const BitAbstractArchiveHandler& ) = delete;

        BitAbstractArchiveHandler( BitAbstractArchiveHandler&& ) = delete;

        auto operator=( const BitAbstractArchiveHandler& ) -> BitAbstractArchiveHandler& = delete;

        auto operator=( BitAbstractArchiveHandler&& ) -> BitAbstractArchiveHandler& = delete;

        virtual ~BitAbstractArchiveHandler() = default;

        /**
         * @return the Bit7zLibrary object used by the handler.
         */
        BIT7Z_NODISCARD auto library() const noexcept -> const Bit7zLibrary&;

        /**
         * @return the format used by the handler for extracting or compressing.
         */
        BIT7Z_NODISCARD virtual auto format() const noexcept -> const BitInFormat& = 0;

        /**
         * @return the password used to open, extract, or encrypt the archive.
         */
        BIT7Z_NODISCARD auto password() const -> const tstring&;

        /**
         * @return a boolean value indicating whether the directory structure must be preserved while extracting
         * or compressing the archive.
         */
        BIT7Z_NODISCARD auto retainDirectories() const noexcept -> bool;

        /**
         * @return a boolean value indicating whether a password is defined or not.
         */
        BIT7Z_NODISCARD auto isPasswordDefined() const noexcept -> bool;

        /**
         * @return the current total callback.
         */
        BIT7Z_NODISCARD auto totalCallback() const -> const TotalCallback&;

        /**
         * @return the current progress callback.
         */
        BIT7Z_NODISCARD auto progressCallback() const -> const ProgressCallback&;

        /**
         * @return the current ratio callback.
         */
        BIT7Z_NODISCARD auto ratioCallback() const -> const RatioCallback&;

        /**
         * @return the current file callback.
         */
        BIT7Z_NODISCARD auto fileCallback() const -> const FileCallback&;

        /**
         * @return the current password callback.
         */
        BIT7Z_NODISCARD auto passwordCallback() const -> const PasswordCallback&;

        /**
         * @return the current OverwriteMode.
         */
        BIT7Z_NODISCARD auto overwriteMode() const noexcept -> OverwriteMode;

        /**
         * @brief Sets up a password to be used by the archive handler.
         *
         * The password will be used to encrypt/decrypt archives by using the default
         * cryptographic method of the archive format.
         *
         * @note Calling setPassword when the input archive is not encrypted does not have any effect on
         * the extraction process.
         *
         * @note Calling setPassword when the output format doesn't support archive encryption
         * (e.g., GZip, BZip2, etc...) does not have any effects (in other words, it doesn't
         * throw exceptions, and it has no effects on compression operations).
         *
         * @note After a password has been set, it will be used for every subsequent operation.
         * To disable the use of the password, you need to call the clearPassword method, which is equivalent
         * to calling setPassword(L"").
         *
         * @param password  the password to be used.
         */
        virtual void setPassword( const tstring& password );

        /**
         * @brief Clear the current password used by the handler.
         *
         * Calling clearPassword() will disable the encryption/decryption of archives.
         *
         * @note This is equivalent to calling setPassword(L"").
         */
        void clearPassword() noexcept;

        /**
         * @brief Sets whether the operations' output will preserve the input's directory structure or not.
         *
         * @param retain  the setting for preserving or not the input directory structure
         */
        void setRetainDirectories( bool retain ) noexcept;

        /**
         * @brief Sets the function to be called when the total size of an operation is available.
         *
         * @param callback  the total callback to be used.
         */
        void setTotalCallback( const TotalCallback& callback );

        /**
         * @brief Sets the function to be called when the processed size of the ongoing operation is updated.
         *
         * @note The completion percentage of the current operation can be obtained by calculating
         * `static_cast<int>((100.0 * processed_size) / total_size)`.
         *
         * @param callback  the progress callback to be used.
         */
        void setProgressCallback( const ProgressCallback& callback );

        /**
         * @brief Sets the function to be called when the input processed size and current output size of the
         * ongoing operation are known.
         *
         * @note The ratio percentage of a compression operation can be obtained by calculating
         * `static_cast<int>((100.0 * output_size) / input_size)`.
         *
         * @param callback  the ratio callback to be used.
         */
        void setRatioCallback( const RatioCallback& callback );

        /**
         * @brief Sets the function to be called when the current file being processed changes.
         *
         * @param callback  the file callback to be used.
         */
        void setFileCallback( const FileCallback& callback );

        /**
         * @brief Sets the function to be called when a password is needed to complete the ongoing operation.
         *
         * @param callback  the password callback to be used.
         */
        void setPasswordCallback( const PasswordCallback& callback );

        /**
         * @brief Sets how the handler should behave when it tries to output to an existing file or buffer.
         *
         * @param mode  the OverwriteMode to be used by the handler.
         */
        void setOverwriteMode( OverwriteMode mode );

    protected:
        explicit BitAbstractArchiveHandler(
            const Bit7zLibrary& lib,
            tstring password = {},
            OverwriteMode overwriteMode = OverwriteMode::None
        );

    private:
        const Bit7zLibrary& mLibrary;
        tstring mPassword;
        bool mRetainDirectories;
        OverwriteMode mOverwriteMode;

        //CALLBACKS
        TotalCallback mTotalCallback;
        ProgressCallback mProgressCallback;
        RatioCallback mRatioCallback;
        FileCallback mFileCallback;
        PasswordCallback mPasswordCallback;
};

} // namespace bit7z

#endif // BITABSTRACTARCHIVEHANDLER_HPP

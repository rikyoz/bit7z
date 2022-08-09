/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef BITABSTRACTARCHIVEHANDLER_HPP
#define BITABSTRACTARCHIVEHANDLER_HPP

#include <cstdint>
#include <functional>

#include "bit7zlibrary.hpp"
#include "bitdefines.hpp"

namespace bit7z {
using std::function;

class BitInFormat;

/**
 * @brief A std::function whose argument is the total size of the ongoing operation.
 */
using TotalCallback = function< void( uint64_t ) >;

/**
 * @brief A std::function whose argument is the currently processed size of the ongoing operation and returns
 *        true or false whether the operation must continue or not.
 */
using ProgressCallback = function< bool( uint64_t ) >;

/**
 * @brief A std::function whose arguments are the current processed input size, and the current output size of the
 *        ongoing operation.
 */
using RatioCallback = function< void( uint64_t, uint64_t ) >;

/**
 * @brief A std::function whose argument is the path, in the archive, of the file currently being processed
 *        by the ongoing operation.
 */
using FileCallback = function< void( tstring ) >;

/**
 * @brief A std::function returning the password to be used to handle an archive.
 */
using PasswordCallback = function< tstring() >;

/**
 * @brief Abstract class representing a generic archive handler.
 */
class BitAbstractArchiveHandler {
    public:
        BitAbstractArchiveHandler( const BitAbstractArchiveHandler& ) = delete;

        BitAbstractArchiveHandler( BitAbstractArchiveHandler&& ) = delete;

        BitAbstractArchiveHandler& operator=( const BitAbstractArchiveHandler& ) = delete;

        BitAbstractArchiveHandler& operator=( BitAbstractArchiveHandler&& ) = delete;

        virtual ~BitAbstractArchiveHandler() = default;

        /**
         * @return the Bit7zLibrary object used by the handler.
         */
        BIT7Z_NODISCARD const Bit7zLibrary& library() const noexcept;

        /**
         * @return the format used by the handler for extracting or compressing.
         */
        BIT7Z_NODISCARD virtual const BitInFormat& format() const = 0;

        /**
         * @return the password used to open, extract, or encrypt the archive.
         */
        BIT7Z_NODISCARD tstring password() const;

        /**
         * @return true if the directory structure must be preserved while extracting or compressing
         *         the archive, false otherwise.
         */
        BIT7Z_NODISCARD bool retainDirectories() const noexcept;

        /**
         * @return true if a password is defined, false otherwise.
         */
        BIT7Z_NODISCARD bool isPasswordDefined() const noexcept;

        /**
         * @return the current total callback.
         */
        BIT7Z_NODISCARD TotalCallback totalCallback() const;

        /**
         * @return the current progress callback.
         */
        BIT7Z_NODISCARD ProgressCallback progressCallback() const;

        /**
         * @return the current ratio callback.
         */
        BIT7Z_NODISCARD RatioCallback ratioCallback() const;

        /**
         * @return the current file callback.
         */
        BIT7Z_NODISCARD FileCallback fileCallback() const;

        /**
         * @return the current password callback.
         */
        BIT7Z_NODISCARD PasswordCallback passwordCallback() const;

        /**
         * @brief Sets up a password to be used by the archive handler.
         *
         * The password will be used to encrypt/decrypt archives by using the default
         * cryptographic method of the archive format.
         *
         * @note Calling setPassword when the input archive is not encrypted does not have effect on
         * the extraction process.
         *
         * @note Calling setPassword when the output format doesn't support archive encryption
         * (e.g., GZip, BZip2, etc...) does not have any effects (in other words, it doesn't
         * throw exceptions, and it has no effects on compression operations).
         *
         * @note After a password has been set, it will be used for every subsequent operation.
         * To disable the use of the password, you need to call the clearPassword method, which is equivalent
         * to call setPassword(L"").
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
         * @brief Sets whether methods output will preserve the input's directory structure or not.
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
         * static_cast<int>( ( 100.0 * processed_size ) / total_size ).
         *
         * @param callback  the progress callback to be used.
         */
        void setProgressCallback( const ProgressCallback& callback );

        /**
         * @brief Sets the function to be called when the input processed size and current output size of the
         * ongoing operation are known.
         *
         * @note The ratio percentage of a compression operation can be obtained by calculating
         * static_cast<int>( ( 100.0 * output_size ) / input_size ).
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

    protected:
        const Bit7zLibrary& mLibrary;
        tstring mPassword;
        bool mRetainDirectories;

        explicit BitAbstractArchiveHandler( const Bit7zLibrary& lib, tstring password = {} );

    private:
        //CALLBACKS
        TotalCallback mTotalCallback;
        ProgressCallback mProgressCallback;
        RatioCallback mRatioCallback;
        FileCallback mFileCallback;
        PasswordCallback mPasswordCallback;
};
}  // namespace bit7z

#endif // BITABSTRACTARCHIVEHANDLER_HPP

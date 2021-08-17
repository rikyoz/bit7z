/*
 * bit7z - A C++ static library to interface with the 7-zip DLLs.
 * Copyright (c) 2014-2021  Riccardo Ostani - All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * Bit7z is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with bit7z; if not, see https://www.gnu.org/licenses/.
 */

#ifndef BITABSTRACTARCHIVECREATOR_HPP
#define BITABSTRACTARCHIVECREATOR_HPP

#include <memory>

#include "bitabstractarchivehandler.hpp"
#include "bitcompressionlevel.hpp"
#include "bitcompressionmethod.hpp"
#include "bitformat.hpp"
#include "bitinputarchive.hpp"

struct IOutStream;
struct ISequentialOutStream;

namespace bit7z {
    using std::unique_ptr;
    using std::ostream;

    struct ArchiveProperties {
        vector< const wchar_t* > names;
        vector< BitPropVariant > values;
    };

    enum struct UpdateMode {
        None,
        Append,
        Overwrite
    };

    /**
     * @brief Abstract class representing a generic archive creator.
     */
    class BitAbstractArchiveCreator : public BitAbstractArchiveHandler {
        public:
            BitAbstractArchiveCreator( const BitAbstractArchiveCreator& ) = delete;

            BitAbstractArchiveCreator( BitAbstractArchiveCreator&& ) = delete;

            BitAbstractArchiveCreator& operator=( const BitAbstractArchiveCreator& ) = delete;

            BitAbstractArchiveCreator& operator=( BitAbstractArchiveCreator&& ) = delete;

            /**
             * @return the format used by the archive creator.
             */
            BIT7Z_NODISCARD const BitInFormat& format() const noexcept override;

            /**
             * @return the format used by the archive creator.
             */
            BIT7Z_NODISCARD const BitInOutFormat& compressionFormat() const noexcept;

            /**
             * @return whether the creator crypts also the headers of archives or not
             */
            BIT7Z_NODISCARD bool cryptHeaders() const noexcept;

            /**
             * @return the compression level used by the archive creator.
             */
            BIT7Z_NODISCARD BitCompressionLevel compressionLevel() const noexcept;

            /**
             * @return the compression method used by the archive creator.
             */
            BIT7Z_NODISCARD BitCompressionMethod compressionMethod() const noexcept;

            /**
             * @return the dictionary size used by the archive creator.
             */
            BIT7Z_NODISCARD uint32_t dictionarySize() const noexcept;

            BIT7Z_NODISCARD uint32_t wordSize() const noexcept;

            /**
             * @return whether the archive creator uses solid compression or not.
             */
            BIT7Z_NODISCARD bool solidMode() const noexcept;

            /**
             * @return whether the archive creator is allowed to update existing archives or not.
             */
            BIT7Z_NODISCARD UpdateMode updateMode() const noexcept;

            /**
             * @return the size (in bytes) of the archive volume used by the creator
             *         (a 0 value means that all files are going in a single archive).
             */
            BIT7Z_NODISCARD uint64_t volumeSize() const noexcept;

            BIT7Z_NODISCARD uint32_t threadsCount() const noexcept;

            /**
             * @brief Sets up a password for the output archive.
             *
             * When setting a password, the produced archive will be encrypted using the default
             * cryptographic method of the output format. The option "crypt headers" remains unchanged,
             * in contrast with what happens when calling the setPassword(tstring, bool) method.
             *
             * @note Calling setPassword when the output format doesn't support archive encryption
             * (e.g. GZip, BZip2, etc...) does not have any effects (in other words, it doesn't
             * throw exceptions and it has no effects on compression operations).
             *
             * @note After a password has been set, it will be used for every subsequent operation.
             * To disable the use of the password, you need to call the clearPassword method
             * (inherited from BitAbstractArchiveHandler), which is equivalent to setPassword(L"").
             *
             * @param password
             */
            void setPassword( const tstring& password ) override;

            /**
             * @brief Sets up a password for the output archive.
             *
             * When setting a password, the produced archive will be encrypted using the default
             * cryptographic method of the output format. If the format is 7z and the option
             * "crypt_headers" is set to true, also the headers of the archive will be encrypted,
             * resulting in a password request every time the output file will be opened.
             *
             * @note Calling setPassword when the output format doesn't support archive encryption
             * (e.g. GZip, BZip2, etc...) does not have any effects (in other words, it doesn't
             * throw exceptions and it has no effects on compression operations).
             *
             * @note Calling setPassword with "crypt_headers" set to true does not have effects on
             * formats different from 7z.
             *
             * @note After a password has been set, it will be used for every subsequent operation.
             * To disable the use of the password, you need to call the clearPassword method
             * (inherited from BitAbstractArchiveHandler), which is equivalent to setPassword(L"").
             *
             * @param password          the password desired.
             * @param crypt_headers     if true, the headers of the output archive will be encrypted
             *                          (valid only with 7z format).
             */
            void setPassword( const tstring& password, bool crypt_headers );

            /**
             * @brief Sets the compression level to be used when creating an archive.
             *
             * @param level the compression level desired.
             */
            void setCompressionLevel( BitCompressionLevel level ) noexcept;

            /**
             * @brief Sets the compression method to be used when creating an archive.
             *
             * @param method the compression method desired.
             */
            void setCompressionMethod( BitCompressionMethod method );

            /**
             * @brief Sets the dictionary size to be used when creating an archive.
             *
             * @param dictionary_size the dictionary size desired.
             */
            void setDictionarySize( uint32_t dictionary_size );

            void setWordSize( uint32_t word_size );

            /**
             * @brief Sets whether to use solid compression or not.
             *
             * @note Setting the solid compression mode to true has effect only when using the 7z format with multiple
             * input files.
             *
             * @param solid_mode    if true, it will be used the "solid compression" method.
             */
            void setSolidMode( bool solid_mode ) noexcept;

            /**
             * @brief Sets whether the creator can update existing archives or not.
             *
             * @note If false, an exception will be thrown in case a compression operation targets an existing archive.
             *
             * @param mode if true, compressing operations will update existing archives.
             */
            virtual void setUpdateMode( UpdateMode mode );

            /**
             * @brief Sets the volume_size (in bytes) of the archive volumes.
             *
             * @note This setting has effects only when the destination archive is on filesystem.
             *
             * @param volume_size    The dimension of a volume.
             */
            void setVolumeSize( uint64_t volume_size ) noexcept;

            void setThreadsCount( uint32_t threads_count ) noexcept;

        protected:
            const BitInOutFormat& mFormat;

            BitAbstractArchiveCreator( const Bit7zLibrary& lib,
                                       const BitInOutFormat& format,
                                       tstring password = TSTRING( "" ),
                                       UpdateMode update_mode = UpdateMode::None );

            ~BitAbstractArchiveCreator() override = default;

            BIT7Z_NODISCARD ArchiveProperties getArchiveProperties() const;

            friend class BitOutputArchive;

        private:
            UpdateMode mUpdateMode;
            BitCompressionLevel mCompressionLevel;
            BitCompressionMethod mCompressionMethod;
            uint32_t mDictionarySize;
            uint32_t mWordSize;
            bool mCryptHeaders;
            bool mSolidMode;
            uint64_t mVolumeSize;
            uint32_t mThreadsCount;
    };
}

#endif // BITABSTRACTARCHIVECREATOR_HPP

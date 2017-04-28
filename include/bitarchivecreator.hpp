#ifndef BITARCHIVECREATOR_HPP
#define BITARCHIVECREATOR_HPP

#include "../include/bit7zlibrary.hpp"
#include "../include/bitformat.hpp"
#include "../include/bittypes.hpp"
#include "../include/bitcompressionlevel.hpp"
#include "../include/bitarchivehandler.hpp"

namespace bit7z {
    using std::wstring;

    /**
     * @brief Abstract class representing a generic archive creator.
     */
    class BitArchiveCreator : public BitArchiveHandler {
        public:
            /**
             * @brief BitArchiveCreator constructor.
             *
             * @param lib       the 7z library used.
             * @param format    the output archive format.
             */
            BitArchiveCreator( const Bit7zLibrary& lib, const BitInOutFormat& format );

            /**
             * @brief BitArchiveCreator destructor.
             */
            virtual ~BitArchiveCreator() = 0;

            /**
             * @return the format used by the archive creator.
             */
            const BitInOutFormat& compressionFormat();

            /**
             * @return whether the creator crypts also the headers of archives or not
             */
            bool cryptHeaders() const;

            /**
             * @return the compression level used by the archive creator.
             */
            BitCompressionLevel compressionLevel() const;

            /**
             * @return whether the archive creator uses solid compression or not.
             */
            bool solidMode() const;

            /**
             * @return the size (in bytes) of the archive volume used by the creator
             *         (a 0 value means that all files are going in a single archive).
             */
            uint64_t volumeSize() const;

            /**
             * @brief Sets up a password for the output archive.
             *
             * When setting a password, the produced archive will be encrypted using the default
             * cryptographic method of the output format. The option "crypt headers" remains unchanged,
             * in contrast with what happens when calling the setPassword(wstring, bool) method.
             *
             * @note Calling setPassword when the output format doesn't support archive encryption
             * (e.g. GZip, BZip2, etc...) does not have any effects (in other words, it doesn't
             * throw exceptions and it has no effects on compression operations).
             *
             * @note After a password has been set, it will be used for every subsequent operation.
             * To disable the use of the password, you need to call the clearPassword method
             * (inherited from BitArchiveHandler), which is equivalent to setPassword(L"").
             *
             * @param password
             */
            void setPassword( const wstring &password ) override;

            /**
             * @brief Sets up a password for the output archive.
             *
             * When setting a password, the produced archive will be encrypted using the default
             * cryptographic method of the output format. If the format is 7z and the option
             * "crypt_headers" is set to true, also the headers of the archive will be encrypted,
             * resulting in a password request everytime the output file will be opened.
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
             * (inherited from BitArchiveHandler), which is equivalent to setPassword(L"").
             *
             * @param password          the password desired.
             * @param crypt_headers     if true, the headers of the output archive will be encrypted
             *                          (valid only with 7z format).
             */
            void setPassword( const wstring& password, bool crypt_headers );

            /**
             * @brief Sets the compression level to use when creating an archive.
             *
             * @param compression_level the compression level desired.
             */
            void setCompressionLevel( BitCompressionLevel compression_level );

            /**
             * @brief Sets whether to use solid compression or not.
             *
             * @note Setting the solid compression mode to true has effect only when using the 7z format.
             *
             * @param solid_mode    if true, it will be used the "solid compression" method.
             */
            void setSolidMode( bool solid_mode );

            /**
             * @brief Sets the size (in bytes) of the archive volumes.
             *
             * @note This setting has effects only when the destination archive is on filesystem.
             *
             * @param size    The dimension of a volume.
             */
            void setVolumeSize( uint64_t size );

        protected:
            const BitInOutFormat& mFormat;
            BitCompressionLevel mCompressionLevel;
            bool mCryptHeaders;
            bool mSolidMode;
            uint64_t mVolumeSize;
    };
}

#endif // BITARCHIVECREATOR_HPP

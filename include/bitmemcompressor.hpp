#ifndef BITMEMCOMPRESSOR_HPP
#define BITMEMCOMPRESSOR_HPP

#include <iostream>
#include <vector>

#include "../include/bit7zlibrary.hpp"
#include "../include/bitformat.hpp"
#include "../include/bittypes.hpp"
#include "../include/bitcompressionlevel.hpp"

namespace bit7z {
    using std::wstring;
    using std::vector;

    /**
     * @brief The BitMemCompressor class allows to compress memory buffers to the filesystem or to other memory buffers
     *
     * It let decide various properties of the produced archive file, such as the password
     * protection and the compression level desired.
     */
    class BitMemCompressor {
        public:
            /**
             * @brief Constructs a BitMemCompressor object
             *
             * The Bit7zLibrary parameter is needed in order to have access to the functionalities
             * of the 7z DLLs. On the other hand, the BitInOutFormat is required in order to know the
             * format of the output archive.
             *
             * @param lib       the 7z library used.
             * @param format    the output archive format.
             */
            BitMemCompressor( Bit7zLibrary const& lib, BitInOutFormat const& format );

            /**
             * @return The archive format used by the compressor
             */
            const BitInOutFormat& compressionFormat();

            /**
             * @brief Sets up a password for the output archive
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
             * @note After a password has been set, it will be used for every compression operation.
             * To cancel the password, it must be performed a new call to setPassword with argument
             * an empty password.
             *
             * @param password          the password desired.
             * @param crypt_headers     if true, the headers of the output archive will be encrypted
             *                          (valid only with 7z format).
             */
            void setPassword( const wstring& password, bool crypt_headers = false );

            /**
             * @brief Sets the compression level to use when creating an archive
             *
             * @param compression_level the compression level desired.
             */
            void setCompressionLevel( BitCompressionLevel compression_level );

            /**
             * @brief Sets whether to use solid compression or not
             *
             * @note Setting the solid compression mode to true has effect only when using the 7z format.
             *
             * @param solid_mode    if true, it will be used the "solid compression" method.
             */
            void setSolidMode( bool solid_mode );

            /**
             * @brief Compresses the given buffer to an archive on the filesystem
             *
             * @param in_buffer         the buffer to be compressed
             * @param out_archive       the output archive path
             * @param in_buffer_name    (optional) the buffer name used to give a name to the content of the archive
             */
            void compress( const vector< byte_t >& in_buffer, const wstring& out_archive,
                           wstring in_buffer_name = L"" ) const;

            /**
             * @brief Compresses the given input buffer to the output buffer
             *
             * @note If the format of the output archive doesn't support in memory compression, a BitException is thrown
             *
             * @param in_buffer         the buffer to be compressed
             * @param out_buffer        the buffer going to contain the output archive
             * @param in_buffer_name    (optional) the buffer name used to give a name to the content of the archive
             */
            void compress( const vector< byte_t >& in_buffer, vector< byte_t >& out_buffer,
                           wstring in_buffer_name = L"" ) const;

        private:
            const Bit7zLibrary& mLibrary;
            const BitInOutFormat& mFormat;
            BitCompressionLevel mCompressionLevel;
            wstring mPassword;
            bool mCryptHeaders;
            bool mSolidMode;
    };
}
#endif // BITMEMCOMPRESSOR_HPP

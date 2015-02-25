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

    class BitMemCompressor {
        public:
            BitMemCompressor( Bit7zLibrary const& lib, BitOutFormat const& format );

            BitOutFormat compressionFormat();

            void setPassword( const wstring& password, bool crypt_headers = false );

            void setCompressionLevel( BitCompressionLevel compression_level );

            void setSolidMode( bool solid_mode );

            void compress(const vector<byte_t>& in_buffer/*const byte_t* in_buffer, const size_t in_buffer_size*/,
                           const wstring& out_archive, wstring in_buffer_name = L"" ) const;

            void compress( const vector<byte_t>& in_buffer,
                           vector<byte_t>& out_buffer,
                           wstring in_buffer_name = L""/*const byte_t* in_buffer, const size_t in_buffer_size, byte_t** out_buffer,
                           size_t* out_buffer_size*/ ) const;

        private:
            const Bit7zLibrary& mLibrary;
            const BitOutFormat& mFormat;
            BitCompressionLevel mCompressionLevel;
            wstring mPassword;
            bool mCryptHeaders;
            bool mSolidMode;
    };

}

#endif // BITMEMCOMPRESSOR_HPP

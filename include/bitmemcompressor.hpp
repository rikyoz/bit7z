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
            BitMemCompressor( Bit7zLibrary const& lib, BitInOutFormat const& format );

            BitInOutFormat compressionFormat();

            void setPassword( const wstring& password, bool crypt_headers = false );

            void setCompressionLevel( BitCompressionLevel compression_level );

            void setSolidMode( bool solid_mode );

            void compress( const vector< byte_t >& in_buffer, const wstring& out_archive,
                           wstring in_buffer_name = L"" ) const;

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

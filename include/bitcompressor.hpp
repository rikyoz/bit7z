#ifndef BITCOMPRESSOR_HPP
#define BITCOMPRESSOR_HPP

#include <iostream>
#include <vector>

#include "../include/bit7zlibrary.hpp"
#include "../include/bitguids.hpp"

using namespace std;

namespace Bit7z {
    class BitCompressor {
        public:
            BitCompressor( const Bit7zLibrary& lib, BitFormat format );

            void setPassword( const wstring& password, bool crypt_headers = false );
            void setCompressionLevel();
            void enableSolidMode();

            void compress( const vector<wstring>& in_files, const wstring& out_archive );
            void compressFile( const wstring& in_file, const wstring& out_archive );
            void compressFiles( const wstring& in_dir, const wstring& filter,
                                const wstring& out_archive, bool search_subdirs = true );
            void compressDirectory( const wstring& in_dir, const wstring& out_archive,
                                    bool search_subdirs = true );

        private:
            const Bit7zLibrary& mLibrary;
            const BitFormat mFormat;
            wstring mPassword;
            bool mCryptHeaders;
            bool mSolidMode;
    };
}

#endif // BITCOMPRESSOR_HPP

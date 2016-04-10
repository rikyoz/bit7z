#ifndef BITMEMEXTRACTOR_HPP
#define BITMEMEXTRACTOR_HPP

#include <iostream>
#include <vector>

#include "../include/bit7zlibrary.hpp"
#include "../include/bitguids.hpp"
#include "../include/bittypes.hpp"

namespace bit7z {
    using std::wstring;
    using std::vector;

    class BitMemExtractor {
        public:
            BitMemExtractor( const Bit7zLibrary& lib, BitInFormat BitFormat );

            BitInFormat extractionFormat();

            void setPassword( const wstring& password );

            void extract( const vector< byte_t >& in_buffer, const wstring& out_dir = L"" ) const;

            void extract( const vector< byte_t >& in_buffer, vector< byte_t >& out_buffer,
                          unsigned int index = 0 ) const;

        private:
            const Bit7zLibrary& mLibrary;
            const BitInFormat mFormat;
            wstring mPassword;
    };
}

#endif // BITMEMEXTRACTOR_HPP

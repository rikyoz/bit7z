#ifndef BITEXTRACTOR_HPP
#define BITEXTRACTOR_HPP

#include <iostream>

#include "../include/bit7zlibrary.hpp"
#include "../include/bitguids.hpp"

namespace bit7z {

    class BitExtractor {
        public:
            BitExtractor( const Bit7zLibrary&, BitInFormat format );

            void extract( const std::wstring& in_file, const std::wstring& out_dir = L"",
                          const std::wstring& password = L"" ) const;

        private:
            const Bit7zLibrary& mLibrary;
            const BitInFormat& mFormat;
    };

}

#endif // BITEXTRACTOR_HPP

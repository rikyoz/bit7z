#ifndef BITEXTRACTOR_HPP
#define BITEXTRACTOR_HPP

#include <iostream>

#include "../include/bit7zlibrary.hpp"
#include "../include/bitguids.hpp"

namespace Bit7z {
    class BitExtractor {
        public:
            BitExtractor( const Bit7zLibrary&, BitFormat format );

            void extract( const std::wstring&, const std::wstring& = L"", const std::wstring& = L"" );

        private:
            const Bit7zLibrary& lib;
            const BitFormat format;
    };
}

#endif // BITEXTRACTOR_HPP

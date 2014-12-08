#ifndef BITEXTRACTOR_HPP
#define BITEXTRACTOR_HPP

#include <iostream>

#include "../include/bit7zlibrary.hpp"
#include "../include/bitguids.hpp"

namespace Bit7z {
    class BitExtractor {
        public:
            BitExtractor( const Bit7zLibrary&, BitFormat format );

            void extract( const std::wstring& in_file, const std::wstring& out_dir = L"",
                          const std::wstring& password = L"" );

        private:
            const Bit7zLibrary& mLibrary;
            const BitFormat mFormat;
    };
}

#endif // BITEXTRACTOR_HPP

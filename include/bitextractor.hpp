#ifndef BITEXTRACTOR_HPP
#define BITEXTRACTOR_HPP

#include <iostream>

#include "../include/bit7zlibrary.hpp"
#include "../include/bitguids.hpp"

namespace bit7z {

    /**
     * @brief The BitExtractor class allows to extract the content of various file archives
     */
    class BitExtractor {
        public:
            /**
             * @brief Constructs a BitExtractor object
             *
             * The Bit7zLibrary parameter is needed in order to have access to the functionalities
             * of the 7z DLLs. On the other hand, the BitInFormat is required in order to know the
             * format of the input archive.
             *
             * @param lib       the 7z library used.
             * @param format    the input archive format.
             */
            BitExtractor( const Bit7zLibrary&, BitInFormat format );

            /**
             * @brief Extracts the given archive into the choosen directory

             * @param in_file   the input archive file
             * @param out_dir   the output directory where extracted files will be put
             * @param password  the (eventual) password needed to open/extract the archive
             */
            void extract( const std::wstring& in_file, const std::wstring& out_dir = L"",
                          const std::wstring& password = L"" ) const;

        private:
            const Bit7zLibrary& mLibrary;
            const BitInFormat& mFormat;
    };

}

#endif // BITEXTRACTOR_HPP

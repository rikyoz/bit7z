#ifndef BITEXTRACTOR_HPP
#define BITEXTRACTOR_HPP

#include <iostream>
#include <vector>

#include "../include/bit7zlibrary.hpp"
#include "../include/bitguids.hpp"
#include "../include/bittypes.hpp"
#include "../include/bitarchiveopener.hpp"

namespace bit7z {
    using std::wstring;
    using std::vector;

    /**
     * @brief The BitExtractor class allows to extract the content of file archives.
     */
    class BitExtractor : public BitArchiveOpener {
        public:
            /**
             * @brief Constructs a BitExtractor object.
             *
             * The Bit7zLibrary parameter is needed in order to have access to the functionalities
             * of the 7z DLLs. On the other hand, the BitInFormat is required in order to know the
             * format of the input archive.
             *
             * @param lib       the 7z library used.
             * @param format    the input archive format.
             */
            BitExtractor( const Bit7zLibrary& lib, const BitInFormat& format );

            /**
             * @brief Extracts the given archive into the choosen directory.

             * @param in_file   the input archive file.
             * @param out_dir   the output directory where extracted files will be put.
             */
            void extract( const wstring& in_file, const wstring& out_dir = L"" ) const;

            /**
             * @brief Extracts the given archive into the output buffer.

             * @param in_file      the input archive file.
             * @param out_buffer   the output buffer where the content of the archive will be put.
             * @param index        the index of the file to be extracted from in_file.
             */
            void extract( const wstring& in_file, vector< byte_t >& out_buffer, unsigned int index = 0 );
    };
}
#endif // BITEXTRACTOR_HPP

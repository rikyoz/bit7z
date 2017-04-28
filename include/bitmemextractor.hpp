#ifndef BITMEMEXTRACTOR_HPP
#define BITMEMEXTRACTOR_HPP

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
     * @brief The BitMemExtractor class allows to extract the content of in-memory archives.
     */
    class BitMemExtractor : public BitArchiveOpener {
        public:
            /**
             * @brief Constructs a BitMemExtractor object.
             *
             * The Bit7zLibrary parameter is needed in order to have access to the functionalities
             * of the 7z DLLs. On the other hand, the BitInFormat is required in order to know the
             * format of the input archive.
             *
             * @param lib       the 7z library used.
             * @param format    the input archive format.
             */
            BitMemExtractor( const Bit7zLibrary& lib, const BitInFormat& format );

            /**
             * @brief Extracts the given buffer archive into the choosen directory.
             *
             * @param in_buffer     the buffer containing the archive to be extracted.
             * @param out_dir       the output directory where to put the file extracted.
             */
            void extract( const vector< byte_t >& in_buffer, const wstring& out_dir = L"" ) const;

            /**
             * @brief Extracts the given buffer archive into the output buffer.
             *
             * @param in_buffer    the buffer containing the archive to be extracted.
             * @param out_buffer   the output buffer where the content of the archive will be put.
             * @param index        the index of the file to be extracted from in_buffer.
             */
            void extract( const vector< byte_t >& in_buffer, vector< byte_t >& out_buffer,
                          unsigned int index = 0 ) const;
    };
}

#endif // BITMEMEXTRACTOR_HPP

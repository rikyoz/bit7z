#ifndef BITSTREAMCOMPRESSOR_HPP
#define BITSTREAMCOMPRESSOR_HPP

#include <istream>

#include "../include/bitarchivecreator.hpp"

namespace bit7z {
    using std::istream;

    class BitStreamCompressor : public BitArchiveCreator {
        public:
            /**
             * @brief Constructs a BitStreamCompressor object.
             *
             * The Bit7zLibrary parameter is needed in order to have access to the functionalities
             * of the 7z DLLs. On the other hand, the BitInOutFormat is required in order to know the
             * format of the input archive.
             *
             * @param lib       the 7z library used.
             * @param format    the input archive format.
             */
            BitStreamCompressor( const Bit7zLibrary& lib, const BitInOutFormat& format );

            void compress( istream& in_stream, ostream& out_stream, const wstring& in_stream_name = L"" ) const;

            void compress( istream& in_stream, vector< byte_t >& out_buffer, const wstring& in_stream_name = L"" ) const;

            void compress( istream& in_stream, const wstring& out_file, const wstring& in_stream_name = L"" ) const;
    };
}

#endif // BITSTREAMCOMPRESSOR_HPP

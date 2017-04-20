#ifndef BITARCHIVEOPENER_HPP
#define BITARCHIVEOPENER_HPP

#include "../include/bit7zlibrary.hpp"
#include "../include/bitarchivehandler.hpp"

namespace bit7z {
    class BitArchiveOpener : public BitArchiveHandler {
        public:
            BitArchiveOpener( const Bit7zLibrary& lib, const BitInFormat& format );

            virtual ~BitArchiveOpener() = 0;

            /**
             * @return the archive format used by the archive opener
             */
            const BitInFormat& extractionFormat();

        protected:
            const BitInFormat& mFormat;
    };
}

#endif // BITARCHIVEOPENER_HPP

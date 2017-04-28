#ifndef BITARCHIVEOPENER_HPP
#define BITARCHIVEOPENER_HPP

#include "../include/bit7zlibrary.hpp"
#include "../include/bitarchivehandler.hpp"

namespace bit7z {
    /**
     * @brief Abstract class representing a generic archive opener.
     */
    class BitArchiveOpener : public BitArchiveHandler {
        public:
            /**
             * @brief BitArchiveOpener constructor.
             *
             * @param lib       the 7z library used.
             * @param format    the input archive format.
             */
            BitArchiveOpener( const Bit7zLibrary& lib, const BitInFormat& format );

            /**
             * @brief BitArchiveOpener destructor.
             */
            virtual ~BitArchiveOpener() = 0;

            /**
             * @return the archive format used by the archive opener.
             */
            const BitInFormat& extractionFormat();

        protected:
            const BitInFormat& mFormat;
    };
}

#endif // BITARCHIVEOPENER_HPP

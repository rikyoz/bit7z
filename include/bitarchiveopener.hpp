#ifndef BITARCHIVEOPENER_HPP
#define BITARCHIVEOPENER_HPP

#include "../include/bit7zlibrary.hpp"

namespace bit7z {
    using std::wstring;

    class BitArchiveOpener {
        public:
            BitArchiveOpener( const Bit7zLibrary& lib, const BitInFormat& format );

            virtual ~BitArchiveOpener() = 0;

            /**
             * @return the archive format used by the archive opener
             */
            const BitInFormat& extractionFormat();

            /**
             * @brief Sets up a password to be used to open the archives
             *
             * When setting a password, the opened archive will be decrypted using the default
             * cryptographic method of the input format.
             *
             * @note Calling setPassword when the input archive is not encrypted does not have effect on
             * the extraction process.
             *
             * @note After a password has been set, it will be used for every extraction operation.
             * To cancel the password, it must be performed a new call to setPassword with argument
             * an empty password.
             *
             * @param password          the password to be used.
             */
            void setPassword( const wstring& password );

        protected:
            const Bit7zLibrary& mLibrary;
            const BitInFormat& mFormat;
            wstring mPassword;
    };
}

#endif // BITARCHIVEOPENER_HPP

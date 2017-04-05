#ifndef BITARCHIVEOPENER_HPP
#define BITARCHIVEOPENER_HPP

#include "../include/bit7zlibrary.hpp"
#include "../include/bitarchivehandler.hpp"

namespace bit7z {
    using std::wstring;

    class BitArchiveOpener : public BitArchiveHandler {
        public:
            BitArchiveOpener( const Bit7zLibrary& lib, const BitInFormat& format );

            virtual ~BitArchiveOpener() = 0;

            /**
             * @return the archive format used by the archive opener
             */
            const BitInFormat& extractionFormat();

            /**
             * @return the password used to open the archive
             */
            const wstring password() const;

            /**
             * @return true if a password is defined, false otherwise
             */
            bool isPasswordDefined() const;

            /**
             * @brief Sets up a password to be used to open the archives
             *
             * When setting a password, the opened archive will be decrypted using the default
             * cryptographic method of the input format.
             *
             * @note Calling setPassword when the input archive is not encrypted does not have effect on
             * the extraction process.
             *
             * @note After a password has been set, it will be used for every subsequent operation.
             * To cancel the password, it must be performed a new call to setPassword with an empty password
             * as argument.
             *
             * @param password    the password to be used.
             */
            void setPassword( const wstring& password );

        protected:
            const Bit7zLibrary& mLibrary;
            const BitInFormat& mFormat;
            wstring mPassword;
    };
}

#endif // BITARCHIVEOPENER_HPP

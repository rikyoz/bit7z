#ifndef BITEXTRACTOR_HPP
#define BITEXTRACTOR_HPP

#include <iostream>

#include "../include/bit7zlibrary.hpp"
#include "../include/bitguids.hpp"

namespace bit7z {

    using std::wstring;
    using std::vector;

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
            BitExtractor( const Bit7zLibrary&, const BitInFormat& format );

            /**
             * @return the archive format used by the extractor
             */
            BitInFormat extractionFormat();

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

            /**
             * @brief Extracts the given archive into the choosen directory

             * @param in_file   the input archive file
             * @param out_dir   the output directory where extracted files will be put
             */
            void extract( const wstring& in_file, const wstring& out_dir = L"" ) const;

        private:
            const Bit7zLibrary& mLibrary;
            const  BitInFormat& mFormat;
            wstring  mPassword;
    };

}

#endif // BITEXTRACTOR_HPP

#ifndef BITARCHIVEHANDLER_HPP
#define BITARCHIVEHANDLER_HPP

#include <cstdint>
#include <functional>

#include "../include/bit7zlibrary.hpp"

namespace bit7z {

    using std::wstring;
    using std::function;

    typedef function<void( uint64_t )> TotalCallback;
    typedef function<void( uint64_t )> ProgressCallback;
    typedef function<void( uint64_t, uint64_t )> RatioCallback;
    typedef function<void( wstring )> FileCallback;
    typedef function<wstring()> PasswordCallback;

    class BitArchiveHandler {
        public:
            BitArchiveHandler( const Bit7zLibrary& lib );
            virtual ~BitArchiveHandler() = 0;

            /**
             * @return the password used to open, extract or encrypt the archive
             */
            const wstring password() const;

            /**
             * @return true if a password is defined, false otherwise
             */
            bool isPasswordDefined() const;

            TotalCallback totalCallback() const;
            ProgressCallback progressCallback() const;
            RatioCallback ratioCallback() const;
            FileCallback fileCallback() const;
            PasswordCallback passwordCallback() const;

            /**
             * @brief Sets up a password to be used by the archive handler
             *
             * The password will be used to encrypt/decrypt archives by using the default
             * cryptographic method of the archive format.
             *
             * @note Calling setPassword when the input archive is not encrypted does not have effect on
             * the extraction process.
             *
             * @note Calling setPassword when the output format doesn't support archive encryption
             * (e.g. GZip, BZip2, etc...) does not have any effects (in other words, it doesn't
             * throw exceptions and it has no effects on compression operations).
             *
             * @note After a password has been set, it will be used for every subsequent operation.
             * To cancel the password, it must be performed a new call to setPassword with an empty password
             * as argument.
             *
             * @param password    the password to be used.
             */
            virtual void setPassword( const wstring& password );

            void setTotalCallback( TotalCallback callback );
            void setProgressCallback( ProgressCallback callback );
            void setRatioCallback( RatioCallback callback );
            void setFileCallback( FileCallback callback );
            void setPasswordCallback( PasswordCallback callback );

        protected:
            const Bit7zLibrary& mLibrary;
            wstring mPassword;

            TotalCallback mTotalCallback;
            ProgressCallback mProgressCallback;
            RatioCallback mRatioCallback;
            FileCallback mFileCallback;
            PasswordCallback mPasswordCallback;
    };
}

#endif // BITARCHIVEHANDLER_HPP

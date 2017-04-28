#ifndef BITARCHIVEHANDLER_HPP
#define BITARCHIVEHANDLER_HPP

#include <cstdint>
#include <functional>

#include "../include/bit7zlibrary.hpp"

namespace bit7z {
    using std::wstring;
    using std::function;

    /**
     * @brief A std::function whose argument is the total size of the ongoing operation.
     */
    typedef function<void( uint64_t total_size )> TotalCallback;

    /**
     * @brief A std::function whose argument is the current processed size of the ongoing operation.
     */
    typedef function<void( uint64_t progress_size )> ProgressCallback;

    /**
     * @brief A std::function whose arguments are the current processed input size and the current output size of the
     * ongoing operation.
     */
    typedef function<void( uint64_t input_size, uint64_t output_size )> RatioCallback;

    /**
     * @brief A std::function whose argument is the name of the file currently being processed by the ongoing operation.
     */
    typedef function<void( wstring filename )> FileCallback;

    /**
     * @brief A std::functions which returns the password to be used in order to handle an archive.
     */
    typedef function<wstring()> PasswordCallback;

    /**
     * @brief Abstract class representing a generic archive handler.
     */
    class BitArchiveHandler {
        public:

            /**
             * @brief BitArchiveHandler constructor
             *
             * @param lib   the 7z library used by the handler.
             */
            BitArchiveHandler( const Bit7zLibrary& lib );

            /**
             * @brief BitArchiveHandler destructor
             */
            virtual ~BitArchiveHandler() = 0;

            /**
             * @return the password used to open, extract or encrypt the archive.
             */
            const wstring password() const;

            /**
             * @return true if a password is defined, false otherwise.
             */
            bool isPasswordDefined() const;

            /**
             * @return the current total callback.
             */
            TotalCallback totalCallback() const;

            /**
             * @return the current progress callback.
             */
            ProgressCallback progressCallback() const;

            /**
             * @return the current ratio callback.
             */
            RatioCallback ratioCallback() const;

            /**
             * @return the current file callback.
             */
            FileCallback fileCallback() const;

            /**
             * @return the current password callback.
             */
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
             * To disable the use of the password, you need to call the clearPassword method, which is equivalent
             * to call setPassword(L"").
             *
             * @param password  the password to be used.
             */
            virtual void setPassword( const wstring& password );

            /**
             * @brief Clear the current password used by the handler.
             *
             * Calling clearPassword() will disable the encryption/decryption of archives.
             *
             * @note This is equivalent to calling setPassword(L"").
             */
            void clearPassword();

            /**
             * @brief Sets the callback to be called when the total size of an operation is available.
             *
             * @param callback  the total callback to be used.
             */
            void setTotalCallback( TotalCallback callback );

            /**
             * @brief Sets the callback to be called when the processed size of the ongoing operation is updated.
             *
             * @note The percentage of completition of the current operation can be obtained by calculating
             * static_cast<int>( ( 100.0 * processed_size ) / total_size ).
             *
             * @param callback  the progress callback to be used.
             */
            void setProgressCallback( ProgressCallback callback );

            /**
             * @brief Sets the callback to be called when the input processed size and current output size of the
             * ongoing operation are known.
             *
             * @note The ratio percentage of a compression operation can be obtained by calculating
             * static_cast<int>( ( 100.0 * output_size ) / input_size ).
             *
             * @param callback  the ratio callback to be used.
             */
            void setRatioCallback( RatioCallback callback );

            /**
             * @brief Sets the callback to be called when the currently file being processed changes.
             *
             * @param callback  the file callback to be used.
             */
            void setFileCallback( FileCallback callback );

            /**
             * @brief Sets the callback to be called when a password is needed to complete the ongoing operation.
             *
             * @param callback  the password callback to be used.
             */
            void setPasswordCallback( PasswordCallback callback );

        protected:
            const Bit7zLibrary& mLibrary;
            wstring mPassword;

            //CALLBACKS
            TotalCallback mTotalCallback;
            ProgressCallback mProgressCallback;
            RatioCallback mRatioCallback;
            FileCallback mFileCallback;
            PasswordCallback mPasswordCallback;
    };
}

#endif // BITARCHIVEHANDLER_HPP

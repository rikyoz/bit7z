#ifndef BITARCHIVEHANDLER_HPP
#define BITARCHIVEHANDLER_HPP

#include <cstdint>
#include <functional>

namespace bit7z {

    typedef std::function<void(uint64_t)> TotalCallback;
    typedef std::function<void(uint64_t)> ProgressCallback;
    typedef std::function<void(uint64_t, uint64_t)> RatioCallback;
    typedef std::function<void(std::wstring)> FileCallback;
    typedef std::function<std::wstring()> PasswordCallback;

    class BitArchiveHandler {
        public:
            BitArchiveHandler();
            virtual ~BitArchiveHandler() = 0;

            TotalCallback totalCallback() const;
            ProgressCallback progressCallback() const;
            RatioCallback ratioCallback() const;
            FileCallback fileCallback() const;
            PasswordCallback passwordCallback() const;

            void setTotalCallback( TotalCallback callback );
            void setProgressCallback( ProgressCallback callback );
            void setRatioCallback( RatioCallback callback );
            void setFileCallback( FileCallback callback );
            void setPasswordCallback( PasswordCallback callback );

        protected:
            TotalCallback mTotalCallback;
            ProgressCallback mProgressCallback;
            RatioCallback mRatioCallback;
            FileCallback mFileCallback;
            PasswordCallback mPasswordCallback;
    };
}

#endif // BITARCHIVEHANDLER_HPP

#ifndef BITARCHIVEHANDLER_HPP
#define BITARCHIVEHANDLER_HPP

#include <functional>

namespace bit7z {
    class BitArchiveHandler {
        public:
            BitArchiveHandler();
            virtual ~BitArchiveHandler() = 0;

            std::function<void(uint64_t)> totalCallback() const;
            std::function<void(uint64_t)> progressCallback() const;
            std::function<void(uint64_t, uint64_t)> ratioCallback() const;
            std::function<void(std::wstring)> fileCallback() const;

            void setTotalCallback( std::function<void(uint64_t)> callback );
            void setProgressCallback( std::function<void(uint64_t)> callback );
            void setRatioCallback( std::function<void(uint64_t, uint64_t)> callback );
            void setFileCallback( std::function<void(std::wstring)> callback );

        protected:
            std::function<void(uint64_t)> mTotalCallback;
            std::function<void(uint64_t)> mProgressCallback;
            std::function<void(uint64_t, uint64_t)> mRatioCallback;
            std::function<void(std::wstring)> mFileCallback;
    };
}

#endif // BITARCHIVEHANDLER_HPP

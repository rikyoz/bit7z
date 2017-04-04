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

            void setTotalCallback( std::function<void(uint64_t)> callback );
            void setProgressCallback( std::function<void(uint64_t)> callback );

        protected:
            std::function<void(uint64_t)> mTotalCallback;
            std::function<void(uint64_t)> mProgressCallback;
    };
}

#endif // BITARCHIVEHANDLER_HPP

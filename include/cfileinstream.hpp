#ifndef CFILEINSTREAM_HPP
#define CFILEINSTREAM_HPP

#include "../include/bittypes.hpp"
#include "../include/cstdinstream.hpp"

#include <fstream>

namespace bit7z {
    class CFileInStream : public CStdInStream {
        public:
            CFileInStream( const std::wstring& filePath );
            void open( const std::wstring& filePath );
            bool fail();

        private:
            std::ifstream mFileStream;
    };
}

#endif // CFILEINSTREAM_HPP

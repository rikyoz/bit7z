#ifndef CFILEINSTREAM_HPP
#define CFILEINSTREAM_HPP

#include "../include/bittypes.hpp"
#include "../include/cstdinstream.hpp"
#include "../include/fs.hpp"

namespace bit7z {
    class CFileInStream : public CStdInStream {
        public:
            CFileInStream( const std::wstring& filePath );
            void open( const std::wstring& filePath );
            bool fail();

        private:
            fs::ifstream mFileStream;
    };
}

#endif // CFILEINSTREAM_HPP

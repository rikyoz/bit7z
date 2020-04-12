#ifndef CFILEINSTREAM_HPP
#define CFILEINSTREAM_HPP

#include "../include/bittypes.hpp"
#include "../include/cstdinstream.hpp"
#include "../include/fs.hpp"

namespace bit7z {
    class CFileInStream : public CStdInStream {
        public:
            explicit CFileInStream( const fs::path& filePath );
            void open( const fs::path& filePath );
            bool fail();

        private:
            fs::ifstream mFileStream;
    };
}

#endif // CFILEINSTREAM_HPP

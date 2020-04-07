#ifndef CFILEOUTSTREAM_HPP
#define CFILEOUTSTREAM_HPP

#include "../include/bittypes.hpp"
#include "../include/cstdoutstream.hpp"
#include "../include/fs.hpp"

namespace bit7z {
    class CFileOutStream : public CStdOutStream {
        public:
            CFileOutStream( const std::wstring& filePath, bool createAlways = false );
            void open( const std::wstring& filePath, bool createAlways = false );

            bool fail();

        private:
            fs::ofstream mFileStream;
    };
}

#endif // CFILEOUTSTREAM_HPP

#ifndef FSINDEXER_HPP
#define FSINDEXER_HPP

#include <iostream>
#include <vector>

#include "../include/fsitem.hpp"

namespace bit7z {
    namespace filesystem {

        class FSIndexer {
            public:
                FSIndexer( const std::wstring& directory, const std::wstring& filter = L"*" );
                void listFilesInDirectory( std::vector<FSItem>& result, bool recursive = true );

            private:
                std::wstring mDirectory;
                std::wstring mDirName;
                std::wstring mFilter;
                void listFilesInDirectory( std::vector<FSItem>& result, bool recursive, const std::wstring& prefix );
        };

    }
}
#endif // FSINDEXER_HPP

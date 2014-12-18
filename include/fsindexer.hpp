#ifndef FSINDEXER_HPP
#define FSINDEXER_HPP

#include <iostream>
#include <vector>

#include "../include/fsitem.hpp"

using namespace std;

namespace bit7z {
    namespace filesystem {

        class FSIndexer {
            public:
                FSIndexer( const wstring& directory, const wstring& filter = L"*" );
                void listFilesInDirectory( vector<FSItem>& result, bool recursive = true );

            private:
                wstring mDirectory;
                wstring mDirName;
                wstring mFilter;
                void listFilesInDirectory( vector<FSItem>& result, bool recursive, const wstring& prefix );
        };

    }
}
#endif // FSINDEXER_HPP

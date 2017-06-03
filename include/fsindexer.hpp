#ifndef FSINDEXER_HPP
#define FSINDEXER_HPP

#include <iostream>
#include <vector>

#include "../include/fsitem.hpp"

namespace bit7z {
    namespace filesystem {
        using std::wstring;
        using std::vector;

        class FSIndexer {
            public:
                FSIndexer( const wstring& directory, const wstring& filter = L"*" );
                vector< FSItem > listFilesInDirectory( bool recursive = true );

                static vector< FSItem > listFiles( const vector< wstring >& in_paths );
                static vector< FSItem > removeListedDirectories( const vector< wstring >& in_paths );

            private:
                wstring mDirectory;
                wstring mDirName;
                wstring mFilter;

                void listFilesInDirectory( vector< FSItem >& result, bool recursive, const wstring& prefix );
        };
    }
}
#endif // FSINDEXER_HPP

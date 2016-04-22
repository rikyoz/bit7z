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
                void listFilesInDirectory( vector< FSItem >& result, bool recursive = true );

                static void listFiles( const vector< wstring >& in_paths, vector< FSItem >& out_files );
                static void removeListedDirectories( const vector< wstring >& in_paths, vector< FSItem >& out_files );

            private:
                wstring mDirectory;
                wstring mDirName;
                wstring mFilter;

                void listFilesInDirectory( vector< FSItem >& result, bool recursive, const wstring& prefix );
        };
    }
}
#endif // FSINDEXER_HPP

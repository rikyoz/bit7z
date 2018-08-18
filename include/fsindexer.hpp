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
                static vector< FSItem > indexDirectory( const wstring& in_dir, const wstring& filter = L"", bool recursive = true );
                static vector< FSItem > indexPaths( const vector< wstring >& in_paths, bool recursive = true );

            private:
                FSItem mDirItem;
                wstring mFilter;

                FSIndexer( const wstring& directory, const wstring& filter = L"*" );
                void listDirectoryItems( vector< FSItem >& result, bool recursive, const wstring& prefix = L"" );
        };
    }
}
#endif // FSINDEXER_HPP

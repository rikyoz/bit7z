#ifndef FSINDEXER_HPP
#define FSINDEXER_HPP

#include <iostream>
#include <vector>
#include <map>

#include "../include/fsitem.hpp"

namespace bit7z {
    namespace filesystem {
        using std::wstring;
        using std::vector;
        using std::map;

        class FSIndexer {
            public:
                static vector< FSItem > indexDirectory( const wstring& in_dir, const wstring& filter = L"", bool recursive = true );
                static vector< FSItem > indexPaths( const vector< wstring >& in_paths, bool ignore_dirs = false );
                static vector< FSItem > indexPathsMap( const map<wstring, wstring>& in_paths, bool ignore_dirs = false );

            private:
                FSItem mDirItem;
                wstring mFilter;

                FSIndexer( const wstring& directory, const wstring& filter = L"" );
                void listDirectoryItems( vector< FSItem >& result, bool recursive, const wstring& prefix = L"" );

                static void indexItem( const FSItem& item, bool ignore_dirs, vector< FSItem >& result );
        };
    }
}
#endif // FSINDEXER_HPP

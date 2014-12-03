#ifndef BITFILESYSTEM_HPP
#define BITFILESYSTEM_HPP

#include <vector>

#include "7zip/Archive/IArchive.h"
#include "Windows/COM.h"

using namespace std;

namespace Bit7z {
    namespace FileSystem {
        typedef WIN32_FIND_DATA FSItemInfo;

        class FSItem {
            public:
                FSItem( const wstring& path , const wstring& prefix = L"" );
                FSItem( const wstring& directory, const wstring& prefix, FSItemInfo data );

                bool isDir();
                UInt64 size();
                FILETIME creationTime();
                FILETIME lastAccessTime();
                FILETIME lastWriteTime();
                wstring name();
                wstring relativePath();
                wstring fullPath();
                UInt32 attributes();

            private:
                wstring mDirectory;
                wstring mPrefix;
                FSItemInfo mFileData;
        };

        class FSIndexer {
            public:
                static void listFilesInDirectory( vector<FSItem>& result, const wstring& directory, const wstring& filter = L"*",
                                           bool recursive = true );

            private:
                static void listFilesInDirectory( vector<FSItem>& result, const wstring& directory, const wstring& filter,
                                           bool recursive, const wstring& prefix );

        };

        bool path_exists( const wstring& path );
        /*void files_in_directory( vector<FSItem>& result, const wstring& directory,
                                 const wstring& filter = L"*", const wstring& prefix = L"",
                                 bool recursive = true );*/
    }
}

#endif // BITFILESYSTEM_HPP

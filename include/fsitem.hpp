#ifndef FSITEM_HPP
#define FSITEM_HPP

#include <iostream>
#include <cinttypes>

#include <Windows.h>

using namespace std;

namespace bit7z {
    namespace filesystem {

        typedef WIN32_FIND_DATA FSItemInfo;

        class FSItem {
            public:
                FSItem( const wstring& path , const wstring& relative_dir = L"" );
                FSItem( const wstring& dir, const wstring& relative_dir, FSItemInfo data );

                bool exists() const;
                bool isDir() const;
                uint64_t size() const;
                FILETIME creationTime() const;
                FILETIME lastAccessTime() const;
                FILETIME lastWriteTime() const;
                wstring name() const;
                wstring relativePath() const;
                wstring fullPath() const;
                uint32_t attributes() const;

            private:
                wstring mDirectory;
                wstring mRelativeDirectory;
                FSItemInfo mFileData;
        };

    }
}

#endif // FSITEM_HPP

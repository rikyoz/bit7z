#ifndef FSITEM_HPP
#define FSITEM_HPP

#include <iostream>
#include <cstdint>

#include <Windows.h>

namespace bit7z {
    namespace filesystem {
        typedef WIN32_FIND_DATA FSItemInfo;

        using std::wstring;

        class FSItem {
            public:
                explicit FSItem( const wstring& path );
                explicit FSItem( const wstring& dir, FSItemInfo data , const wstring& searchPath );

                bool isDots() const;
                bool isDir() const;
                uint64_t size() const;
                FILETIME creationTime() const;
                FILETIME lastAccessTime() const;
                FILETIME lastWriteTime() const;
                wstring name() const;
                wstring path() const;
                wstring inArchivePath() const;
                uint32_t attributes() const;

            private:
                wstring    mPath;
                FSItemInfo mFileData;
                wstring    mSearchPath;
        };
    }
}
#endif // FSITEM_HPP

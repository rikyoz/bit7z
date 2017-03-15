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
                FSItem( const wstring& path, const wstring& relative_dir = L"" );
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
                wstring upDirectory() const;
                uint32_t attributes() const;

            private:
                wstring mDirectory;
                wstring mRelativeDirectory;
                FSItemInfo mFileData;
        };
    }
}
#endif // FSITEM_HPP

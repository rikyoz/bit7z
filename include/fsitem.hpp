#ifndef FSITEM_HPP
#define FSITEM_HPP

#include <iostream>
#include <cinttypes>

#include <Windows.h>

namespace bit7z {
    namespace filesystem {

        typedef WIN32_FIND_DATA FSItemInfo;

        class FSItem {
            public:
                FSItem( const std::wstring& path, const std::wstring& relative_dir = L"" );
                FSItem( const std::wstring& dir, const std::wstring& relative_dir, FSItemInfo data );

                bool exists() const;
                bool isDir() const;
                uint64_t size() const;
                FILETIME creationTime() const;
                FILETIME lastAccessTime() const;
                FILETIME lastWriteTime() const;
                std::wstring name() const;
                std::wstring relativePath() const;
                std::wstring fullPath() const;
                uint32_t attributes() const;

            private:
                std::wstring mDirectory;
                std::wstring mRelativeDirectory;
                FSItemInfo mFileData;
        };

    }
}

#endif // FSITEM_HPP

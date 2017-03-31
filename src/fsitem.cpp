#include "../include/fsitem.hpp"

#include "../include/bitexception.hpp"
#include "../include/fsutil.hpp"

#include <string>

using namespace std;
using namespace bit7z::filesystem;

FSItem::FSItem( const wstring& path, const wstring& relative_dir ) :
    mDirectory( path ),
    mRelativeDirectory( relative_dir ),
    mFileData() {
    bool isdir = fsutil::is_directory( mDirectory );
    if ( isdir && !mDirectory.empty() ) {
        const size_t lastSlashIndex = mDirectory.find_last_of( L"\\/" );
        if ( lastSlashIndex == mDirectory.length() - 1 ) {
            mDirectory.pop_back();
        }
    }
    HANDLE find_handle = FindFirstFile( mDirectory.c_str(), &mFileData );
    if ( find_handle == INVALID_HANDLE_VALUE ) {
        throw BitException( L"Invalid path '" + mDirectory + L"'!" );
    }
    if ( !isdir ) {
        const size_t lastSlashIndex = mDirectory.find_last_of( L"\\/" );
        if ( wstring::npos != lastSlashIndex ) {
            mDirectory.resize( lastSlashIndex );
        } else { //path contains only the file name, hence we consider the current directory
            mDirectory = L".\\";
        }
    }
    FindClose( find_handle );
}

FSItem::FSItem( const wstring& dir, const wstring& relative_dir, FSItemInfo data ) :
    mDirectory( dir ),
    mRelativeDirectory( relative_dir ),
    mFileData( data ) {
    const size_t lastSlashIndex = mDirectory.find_last_of( L"\\/" );
    if ( lastSlashIndex == mDirectory.length() - 1 ) {
        mDirectory.pop_back();
    }
}

bool FSItem::exists() const {
    return mFileData.dwFileAttributes != INVALID_FILE_ATTRIBUTES;
}

bool FSItem::isDir() const {
    return ( mFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) != 0;
}

uint64_t FSItem::size() const {
    ULARGE_INTEGER size;
    size.LowPart = mFileData.nFileSizeLow;
    size.HighPart = mFileData.nFileSizeHigh;
    return size.QuadPart;
}

FILETIME FSItem::creationTime() const {
    return mFileData.ftCreationTime;
}

FILETIME FSItem::lastAccessTime() const {
    return mFileData.ftLastAccessTime;
}

FILETIME FSItem::lastWriteTime() const {
    return mFileData.ftLastWriteTime;
}

wstring FSItem::name() const {
    return mFileData.cFileName;
}

wstring FSItem::relativePath() const {
    if ( mRelativeDirectory.empty() ) {
        return mFileData.cFileName;
    }
    return mRelativeDirectory + L"\\" + mFileData.cFileName;
}

wstring FSItem::fullPath() const {
    return mDirectory + L"\\" + mFileData.cFileName;
}

wstring FSItem::upDirectory() const {
    return mDirectory;
}

uint32_t FSItem::attributes() const {
    return mFileData.dwFileAttributes;
}

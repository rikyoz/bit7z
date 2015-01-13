#include "../include/fsindexer.hpp"

#include "../include/fsutil.hpp"
#include "../include/bitexception.hpp"

using namespace std;
using namespace bit7z::filesystem;

FSIndexer::FSIndexer( const wstring& directory, const wstring& filter ) : mDirectory( directory ),
    mFilter( filter ) {
    const size_t lastSlashIndex = mDirectory.find_last_of( L"\\/" );
    if ( lastSlashIndex == mDirectory.length() - 1 )
        mDirectory.pop_back();
    FSItem dirItem( mDirectory );
    if ( !dirItem.exists() ) throw BitException( L"'" + dirItem.name() + L"' does not exists!" );
    if ( !dirItem.isDir() ) throw BitException( L"'" + dirItem.name() + L"' is not a directory!" );
    mDirName = dirItem.name();
}

void FSIndexer::listFilesInDirectory( vector<FSItem>& result, bool recursive ) {
    FSIndexer::listFilesInDirectory( result, recursive, L"" );
}

void FSIndexer::listFilesInDirectory( vector<FSItem>& result, bool recursive,
                                      const wstring& prefix ) {
    wstring filtered_path = mDirectory + L"\\";
    if ( !prefix.empty() )
        filtered_path += prefix + L"\\";
    filtered_path += mFilter;
    FSItemInfo data;
    HANDLE hFind = FindFirstFile( filtered_path.c_str(), &data );

    if ( INVALID_HANDLE_VALUE == hFind )
        throw BitException( L"Invalid path '" + filtered_path + L"'" );

    do {
        //NOTE: This should definitely be optimized!!
        wstring ndir;
        if ( prefix.empty() )
            ndir = mDirectory;
        else if ( prefix[0] == '\\' || prefix[0] == '/' )
            ndir = mDirectory + prefix;
        else
            ndir = mDirectory + L"\\" + prefix;
        FSItem currentItem = FSItem( ndir, mDirName + prefix, data );
        if ( currentItem.isDir() ) {
            if ( recursive && currentItem.name() != L"." && currentItem.name() != L".." ) {
                //wstring nprefix = ( prefix.empty() ) ? currentItem.name() : prefix + L"\\" + currentItem.name();
                listFilesInDirectory( result, true, prefix + L"\\" + currentItem.name() );
            }
        } else
            result.push_back( currentItem );
    } while ( FindNextFile( hFind, &data ) != 0 );

    FindClose( hFind );
}



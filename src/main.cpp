#include <iostream>
#include <vector>

#include <atlbase.h>

#include "7zip/Archive/IArchive.h"

#include "Common/MyInitGuid.h"
#include "Common/StringConvert.h"

#include "Windows/COM.h"
#include "Windows/DLL.h"
#include "Windows/FileFind.h"

#include "CArchiveOpenCallback.h"
#include "CArchiveExtractCallback.h"
#include "CArchiveUpdateCallback.h"

using namespace std;
using namespace NWindows;

// {23170F69-40C1-278A-1000-000110070000}
const GUID CLSID_CFormat7z = { 0x23170F69, 0x40C1, 0x278A, { 0x10, 0x00, 0x00, 0x01, 0x10, 0x07, 0x00, 0x00 } };

const GUID IID_IInStream      = { 0x23170F69, 0x40C1, 0x278A, { 0x00, 0x00, 0x00, 0x03, 0x00, 0x03, 0x00, 0x00 } };
const GUID IID_IOutStream     = { 0x23170F69, 0x40C1, 0x278A, { 0x00, 0x00, 0x00, 0x03, 0x00, 0x04, 0x00, 0x00 } };
const GUID IID_IStreamGetSize = { 0x23170F69, 0x40C1, 0x278A, { 0x00, 0x00, 0x00, 0x03, 0x00, 0x06, 0x00, 0x00 } };
const GUID IID_IInArchive     = { 0x23170F69, 0x40C1, 0x278A, { 0x00, 0x00, 0x00, 0x06, 0x00, 0x60, 0x00, 0x00 } };
const GUID IID_IOutArchive    = { 0x23170F69, 0x40C1, 0x278A, { 0x00, 0x00, 0x00, 0x06, 0x00, 0xA0, 0x00, 0x00 } };
const GUID IID_IArchiveUpdateCallback2 = { 0x23170F69, 0x40C1, 0x278A, { 0x00, 0x00, 0x00, 0x01, 0x00, 0x04, 0x00, 0x02 } };

static const wstring kDllName = L"7za.dll";

typedef UINT32 ( WINAPI* CreateObjectFunc )(
    const GUID* clsID,
    const GUID* interfaceID,
    void** outObject );

bool archive( const vector<wstring> in_files, const wstring& out_archive, const wstring& password = L"" ) {
    NWindows::NDLL::CLibrary lib;

    if ( !lib.Load( kDllName.c_str() ) ) {
        cerr << "Can not load 7-zip library" << endl;
        return false;
    }

    CreateObjectFunc createObjectFunc = ( CreateObjectFunc )lib.GetProc( "CreateObject" );

    if ( createObjectFunc == 0 ) {
        cerr << "Can not get CreateObject" << endl;
        return false;
    }

    CObjectVector<CDirItem> dirItems;
    int i;

    for ( i = 0; i < in_files.size(); i++ ) {
        CDirItem di;
        UString name = GetUnicodeString( in_files[i].c_str() );
        NFile::NFind::CFileInfoW fi;

        if ( !fi.Find( name ) ) {
            wcerr << UString( L"Can't find file" ) << name << endl;
            return false;
        }

        di.Attrib = fi.Attrib;
        di.Size = fi.Size;
        di.CTime = fi.CTime;
        di.ATime = fi.ATime;
        di.MTime = fi.MTime;
        di.Name = in_files[i].substr( in_files[i].find_last_of(L"/\\") + 1 ).c_str();
        di.FullPath = name;
        dirItems.Add( di );
    }

    COutFileStream* outFileStreamSpec = new COutFileStream;
    CMyComPtr<IOutStream> outFileStream = outFileStreamSpec;

    if ( !outFileStreamSpec->Create( out_archive.c_str(), false ) ) {
        cerr << "can't create archive file" << endl;
        return false;
    }

    CMyComPtr<IOutArchive> outArchive;

    if ( createObjectFunc( &CLSID_CFormat7z, &IID_IOutArchive, ( void** )&outArchive ) != S_OK ) {
        cerr << "Can not get class object" << endl;
        return false;
    }

    CArchiveUpdateCallback* updateCallbackSpec = new CArchiveUpdateCallback;
    CMyComPtr<IArchiveUpdateCallback2> updateCallback( updateCallbackSpec );
    updateCallbackSpec->Init( &dirItems );
    // updateCallbackSpec->PasswordIsDefined = true;
    // updateCallbackSpec->Password = L"1";
    HRESULT result = outArchive->UpdateItems( outFileStream, dirItems.Size(), updateCallback );
    updateCallbackSpec->Finilize();

    if ( result != S_OK ) {
        cerr << "Update Error";
        return false;
    }

    for ( i = 0; i < updateCallbackSpec->FailedFiles.Size(); i++ )
        cerr << endl << ( UString )L"Error for file: " << updateCallbackSpec->FailedFiles[i];

    if ( updateCallbackSpec->FailedFiles.Size() != 0 )
        return false;

    return true;
}

bool extract( const wstring& in_file, const wstring& out_dir, const wstring& password = L"" ) {
    NWindows::NDLL::CLibrary lib;

    if ( !lib.Load( kDllName.c_str() ) ) {
        cerr << "Can not load 7-zip library" << endl;
        return false;
    }

    CreateObjectFunc createObjectFunc = ( CreateObjectFunc )lib.GetProc( "CreateObject" );

    if ( createObjectFunc == 0 ) {
        cerr << "Can not get CreateObject" << endl;
        return false;
    }

    CMyComPtr<IInArchive> archive;

    if ( createObjectFunc( &CLSID_CFormat7z, &IID_IInArchive, ( void** )&archive ) != S_OK ) {
        cerr << "Can not get class object" << endl;
        return false;
    }

    CInFileStream* fileSpec = new CInFileStream;
    CMyComPtr<IInStream> file = fileSpec;

    if ( !fileSpec->Open( in_file.c_str() ) ) {
        cerr << "Can not open archive file" << endl;
        return false;
    }

    CArchiveOpenCallback* openCallbackSpec = new CArchiveOpenCallback;
    CMyComPtr<IArchiveOpenCallback> openCallback( openCallbackSpec );
    openCallbackSpec->PasswordIsDefined = password.size() > 0;

    if ( password.size() > 0 )
        openCallbackSpec->Password = password.c_str();

    if ( archive->Open( file, 0, openCallback ) != S_OK ) {
        cerr << "Can not open archive" << endl;
        return false;
    }

    // Extract command
    CArchiveExtractCallback* extractCallbackSpec = new CArchiveExtractCallback;
    CMyComPtr<IArchiveExtractCallback> extractCallback( extractCallbackSpec );
    extractCallbackSpec->Init( archive, out_dir.c_str() ); // second parameter is output folder path
    extractCallbackSpec->PasswordIsDefined = password.size() > 0;

    if ( password.size() > 0 )
        extractCallbackSpec->Password = password.c_str();

    HRESULT result = archive->Extract( NULL, ( UInt32 )( Int32 )( -1 ), false, extractCallback );

    if ( result != S_OK ) {
        cerr << "Extract Error" << endl;
        return false;
    }

    return true;
}

static const wstring directory       = L"C:\\Users\\Oz\\Documents\\Programmazione\\Qt Projects\\ZTest\\test\\";
static const wstring compressed      = directory + L"NuovoREGX.regx";
static const wstring compressed_p    = directory + L"NuovoREGXP.regx";
static const wstring compressed_out  = directory + L"test.7z";
static const wstring compressed_out2 = directory + L"test2.7z";
static const wstring uncompressed1    = directory + L"temp.xml";
static const wstring uncompressed2    = directory + L"8.pdf";

int main() {
    //extract( compressed, directory, L"qwertyuiop" );
    archive({uncompressed1}, compressed_out);
    archive({uncompressed1, uncompressed2}, compressed_out2);
    return 0;
}


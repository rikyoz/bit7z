#include <iostream>

#include <atlbase.h>

#include "7zip/Archive/IArchive.h"

#include "Common/MyInitGuid.h"

#include "Windows/COM.h"
#include "Windows/DLL.h"

#include "CArchiveOpenCallback.h"
#include "CArchiveExtractCallback.h"

using namespace std;

// {23170F69-40C1-278A-1000-000110070000}
const GUID CLSID_CFormat7z = { 0x23170F69, 0x40C1, 0x278A, { 0x10, 0x00, 0x00, 0x01, 0x10, 0x07, 0x00, 0x00 } };

const GUID IID_IInStream = { 0x23170F69, 0x40C1, 0x278A, { 0x00, 0x00, 0x00, 0x03, 0x00, 0x03, 0x00, 0x00 } };
const GUID IID_IOutStream =	{ 0x23170F69, 0x40C1, 0x278A, { 0x00, 0x00, 0x00, 0x03, 0x00, 0x04, 0x00, 0x00 } };
const GUID IID_IStreamGetSize =	{ 0x23170F69, 0x40C1, 0x278A, { 0x00, 0x00, 0x00, 0x03, 0x00, 0x06, 0x00, 0x00 } };
const GUID IID_IInArchive =	{ 0x23170F69, 0x40C1, 0x278A, { 0x00, 0x00, 0x00, 0x06, 0x00, 0x60, 0x00, 0x00 } };

static const wstring kDllName = L"7za.dll";

typedef UINT32 ( WINAPI* CreateObjectFunc )(
    const GUID* clsID,
    const GUID* interfaceID,
    void** outObject );

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
        return 1;
    }

    CInFileStream* fileSpec = new CInFileStream;
    CMyComPtr<IInStream> file = fileSpec;

    if ( !fileSpec->Open( in_file.c_str() ) ) {
        cerr << "Can not open archive file" << endl;
        return 1;
    }

    CArchiveOpenCallback* openCallbackSpec = new CArchiveOpenCallback;
    CMyComPtr<IArchiveOpenCallback> openCallback( openCallbackSpec );
    openCallbackSpec->PasswordIsDefined = password.size() > 0;

    if ( password.size() > 0 )
        openCallbackSpec->Password = password.c_str();

    if ( archive->Open( file, 0, openCallback ) != S_OK ) {
        cerr << "Can not open archive" << endl;
        return 1;
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
        return 1;
    }

    return false;
}

const wstring compressed = L"C:\\Users\\Oz\\Documents\\Programmazione\\Qt Projects\\ZTest\\NuovoREGXP.regx";
const wstring directory  = L"C:\\Users\\Oz\\Documents\\Programmazione\\Qt Projects\\ZTest\\test\\";

int main() {
    extract( compressed, directory, L"qwertyuiop" );
    return 0;
}


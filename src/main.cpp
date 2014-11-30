#include <iostream>
#include <vector>

#include "bitextractor.hpp"
#include "bitexception.hpp"

/*#include <atlbase.h>

#include "7zip/Archive/IArchive.h"

#include "Common/MyInitGuid.h"
#include "Common/StringConvert.h"

#include "Windows/COM.h"
#include "Windows/DLL.h"
#include "Windows/FileFind.h"

#include "CArchiveOpenCallback.h"
#include "CArchiveExtractCallback.h"
#include "CArchiveUpdateCallback.h"*/

using namespace std;
//using namespace NWindows;

// {23170F69-40C1-278A-1000-000110070000}
/*const GUID CLSID_CFormat7z = { 0x23170F69, 0x40C1, 0x278A, { 0x10, 0x00, 0x00, 0x01, 0x10, 0x07, 0x00, 0x00 } };

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
}*/

static const wstring directory       = L"C:\\Users\\Oz\\Documents\\Programmazione\\Qt Projects\\Bit7z\\test\\";
static const wstring compressed      = directory + L"NuovoREGX.regx";
static const wstring compressed_p    = directory + L"NuovoREGXP.regx";
static const wstring compressed_out  = directory + L"test.7z";
static const wstring compressed_out2 = directory + L"test2.7z";
static const wstring uncompressed1    = directory + L"temp.xml";
static const wstring uncompressed2    = directory + L"8.pdf";

int main() {
    //extract( compressed, directory, L"qwertyuiop" );
    //archive({uncompressed1}, compressed_out);
    //archive({uncompressed1, uncompressed2}, compressed_out2);
    using namespace Bit7z;

    try {
        Bit7zLibrary lib(L"7z.dll");
        BitExtractor extractor(lib, BitFormat::SevenZip);
        extractor.extract(compressed, directory);
        extractor.extract(compressed_p, directory + L"/p/", L"qwertyuiop");
    } catch ( BitException &e ) {
        cerr << "[Errore] " << e.what() << endl;
    } catch ( ... ) {
        cerr << "[Errore] Sconosciuto..." << endl;
    }

    return 0;
}


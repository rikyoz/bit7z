#include "../include/bit7zlibrary.hpp"

#include "../include/bitexception.hpp"
#include "../include/bitguids.hpp"

#define DEFAULT_DLL L"7z.dll"

using namespace Bit7z;

Bit7zLibrary::Bit7zLibrary() : Bit7zLibrary( DEFAULT_DLL ) {}

Bit7zLibrary::Bit7zLibrary( const std::wstring& dll_path ) {
    if ( !mLibrary.Load( dll_path.c_str() ) )
        throw BitException( "Cannot load 7-zip library" );

    mCreateObjectFunc = reinterpret_cast< CreateObjectFunc >( mLibrary.GetProc( "CreateObject" ) );

    if ( mCreateObjectFunc == NULL )
        throw BitException( "Cannot get CreateObject" );
}

CMyComPtr<IInArchive> Bit7zLibrary::inputArchiveObject( BitFormat format ) const {
    CMyComPtr<IInArchive> archiveObj;
    createArchiveObject( format, &IID_IInArchive, ( void** )&archiveObj );
    return archiveObj;
}

CMyComPtr<IOutArchive> Bit7zLibrary::outputArchiveObject( BitFormat format ) const {
    CMyComPtr<IOutArchive> archiveObj;
    createArchiveObject( format, &IID_IOutArchive, ( void** )&archiveObj );
    return archiveObj;
}

void Bit7zLibrary::createArchiveObject( BitFormat format,
                                        const GUID* interface_ID,
                                        void** out_object ) const {
    const GUID* clsID;

    switch ( format ) {
        case BitFormat::Zip:
            clsID = &CLSID_CFormatZip;
            break;
        case BitFormat::BZip2:
            clsID = &CLSID_CFormatBZip2;
            break;
        case BitFormat::SevenZip:
            clsID = &CLSID_CFormat7z;
            break;
        case BitFormat::Rar:
            clsID = &CLSID_CFormatRar;
            break;
        case BitFormat::Cab:
            clsID = &CLSID_CFormatCab;
            break;
        case BitFormat::Lzma:
            clsID = &CLSID_CFormatLzma;
            break;
        case BitFormat::Lzma86:
            clsID = &CLSID_CFormatLzma86;
            break;
        case BitFormat::Iso:
            clsID = &CLSID_CFormatIso;
            break;
        case BitFormat::Tar:
            clsID = &CLSID_CFormatTar;
            break;
        case BitFormat::GZip:
            clsID = &CLSID_CFormatGZip;
            break;
        default:
            throw BitException( "Format not supported!" );
    }

    if ( mCreateObjectFunc( clsID, interface_ID, out_object ) != S_OK )
        throw BitException( "Cannot get class object" );
}

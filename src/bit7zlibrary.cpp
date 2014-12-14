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

CMyComPtr<IInArchive> Bit7zLibrary::inputArchiveObject( BitInFormat format ) const {
    CMyComPtr<IInArchive> archiveObj;
    createArchiveObject( &format.guid(), &IID_IInArchive, reinterpret_cast< void** >( &archiveObj ) );
    return archiveObj;
}

CMyComPtr<IOutArchive> Bit7zLibrary::outputArchiveObject( BitOutFormat format ) const {
    CMyComPtr<IOutArchive> archiveObj;
    createArchiveObject( &format.guid(), &IID_IOutArchive, reinterpret_cast< void** >( &archiveObj ) );
    return archiveObj;
}

void Bit7zLibrary::createArchiveObject( const GUID* format_ID,
                                        const GUID* interface_ID,
                                        void** out_object ) const {
    if ( mCreateObjectFunc( format_ID, interface_ID, out_object ) != S_OK )
        throw BitException( "Cannot get class object" );
}

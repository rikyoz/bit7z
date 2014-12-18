#include "../include/bit7zlibrary.hpp"

#include "../include/bitexception.hpp"
#include "../include/bitguids.hpp"

#define DEFAULT_DLL L"7z.dll"

using namespace bit7z;

Bit7zLibrary::Bit7zLibrary() : Bit7zLibrary( DEFAULT_DLL ) {}

Bit7zLibrary::Bit7zLibrary( const std::wstring& dll_path ) : mLibrary( LoadLibrary(
                dll_path.c_str() ) ) {
    if ( !mLibrary )
        throw BitException( "Cannot load 7-zip library" );

    mCreateObjectFunc = reinterpret_cast< CreateObjectFunc >( GetProcAddress(mLibrary, "CreateObject" ) );

    if ( !mCreateObjectFunc )
        throw BitException( "Cannot get CreateObject" );
}

void Bit7zLibrary::createArchiveObject( const GUID* format_ID,
                                        const GUID* interface_ID,
                                        void** out_object ) const {
    if ( mCreateObjectFunc( format_ID, interface_ID, out_object ) != S_OK )
        throw BitException( "Cannot get class object" );
}

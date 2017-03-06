#include "../include/bit7zlibrary.hpp"

#include "../include/bitexception.hpp"
#include "../include/bitguids.hpp"

#include <sstream>

using namespace bit7z;
using std::ostringstream;

Bit7zLibrary::Bit7zLibrary( const std::wstring &dll_path ) : mLibrary( LoadLibrary( dll_path.c_str() ) ) {
    if ( !mLibrary ) {
        ostringstream os;
        os << GetLastError();
        throw BitException( "Cannot load 7-zip library (error " + os.str() + ")" );
    }

    mCreateObjectFunc = reinterpret_cast< CreateObjectFunc >( GetProcAddress( mLibrary, "CreateObject" ) );

    if ( !mCreateObjectFunc ) {
        ostringstream os;
        os << GetLastError();
        throw BitException( "Cannot get CreateObject (error " + os.str() + ")" );
    }
}

Bit7zLibrary::~Bit7zLibrary() {
    FreeLibrary( mLibrary );
}

void Bit7zLibrary::createArchiveObject( const GUID *format_ID, const GUID *interface_ID, void **out_object ) const {
    if ( mCreateObjectFunc( format_ID, interface_ID, out_object ) != S_OK ) {
        throw BitException( "Cannot get class object" );
    }
}

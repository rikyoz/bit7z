#include "../include/util.hpp"

#ifdef _WIN32
#include <Windows.h>
#else
#include <sstream>

using std::ostringstream;
using std::wostringstream;
using std::ctype;
using std::use_facet;
#endif

using namespace bit7z;

string bit7z::narrow( const wchar_t* wideString, size_t size ) {
#ifdef _WIN32
    int wideSize = static_cast< int >( size );
    int narrowSize = WideCharToMultiByte( CP_UTF8, 0, wideString, wideSize, nullptr, 0, nullptr, nullptr );
    string result;
    if ( narrowSize > 0 ) {
        result.resize( static_cast< size_t >( narrowSize ) );
        WideCharToMultiByte( CP_UTF8, 0, wideString, wideSize, &result[ 0 ], narrowSize, nullptr, nullptr );
    }
    return result;
#else
    ostringstream stream;
    stream.imbue( std::locale( "C" ) );
    const auto& ctfacet = use_facet< ctype< char > >( stream.getloc() ) ;
    for ( size_t i = 0 ; i < size; ++i ) {
        stream << ctfacet.narrow( wideString[ i ], 0 );
    }
    return stream.str();
#endif
}

wstring bit7z::widen( const string& narrowString ) {
#ifdef _WIN32
    int narrowSize = static_cast< int >( narrowString.size() );
    int wideSize = MultiByteToWideChar( CP_UTF8, 0, &narrowString[ 0 ], narrowSize, nullptr, 0 );
    wstring result;
    if ( wideSize > 0 ) {
        result.resize( static_cast< size_t >( wideSize ) );
        MultiByteToWideChar( CP_UTF8, 0, &narrowString[ 0 ], narrowSize, &result[ 0 ], wideSize );
    }
    return result;
#else
    wostringstream stream;
    stream.imbue( std::locale( "" ) );
    const auto& ctfacet = use_facet< ctype< wchar_t > >( stream.getloc() ) ;
    for ( char c : narrowString ) {
        stream << ctfacet.widen( c ) ;
    }
    return stream.str() ;
#endif
}
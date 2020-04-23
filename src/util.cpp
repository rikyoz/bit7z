#ifndef _WIN32
#include "../include/util.hpp"
#include <sstream>

using std::ostringstream;
using std::wostringstream;
using std::ctype;
using std::use_facet;

using namespace bit7z;

string bit7z::narrow( const wchar_t* wideString, size_t size ) {
    ostringstream stream;
    stream.imbue( std::locale( "C" ) );
    const auto& ctfacet = use_facet< ctype< char > >( stream.getloc() ) ;
    for ( size_t i = 0 ; i < size; ++i ) {
        stream << ctfacet.narrow( wideString[ i ], 0 );
    }
    return stream.str();
}

wstring bit7z::widen( const string& narrowString ) {
    wostringstream stream;
    stream.imbue( std::locale( "" ) );
    const auto& ctfacet = use_facet< ctype< wchar_t > >( stream.getloc() ) ;
    for ( char c : narrowString ) {
        stream << ctfacet.widen( c ) ;
    }
    return stream.str() ;
}
#endif
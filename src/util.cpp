#include "../include/util.hpp"

#include <vector>

#include "Windows/COM.h"
#include "Windows/PropVariant.h"

#include "../include/bitexception.hpp"

using std::vector;
using namespace NWindows;

namespace bit7z {
    namespace util {
        CMyComPtr< IOutArchive > initOutArchive( const Bit7zLibrary &lib, const BitInOutFormat &format,
                                                 const BitCompressionLevel compressionLevel,
                                                 const bool cryptHeaders, const bool solidMode ) {
            CMyComPtr< IOutArchive > outArchive;
            const GUID formatGUID = format.guid();
            lib.createArchiveObject( &formatGUID, &::IID_IOutArchive, reinterpret_cast< void** >( &outArchive ) );

            vector< const wchar_t * >    names;
            vector< NCOM::CPropVariant > values;
            if ( cryptHeaders && format.hasFeature( HEADER_ENCRYPTION ) ) {
                names.push_back( L"he" );
                values.push_back( true );
            }
            if ( format.hasFeature( COMPRESSION_LEVEL ) ) {
                names.push_back( L"x" );
                values.push_back( static_cast< UInt32 >( compressionLevel ) );
            }
            if ( solidMode && format.hasFeature( SOLID_ARCHIVE ) ) {
                names.push_back( L"s" );
                values.push_back( solidMode );
            }

            if ( names.size() > 0 ) {
                CMyComPtr< ISetProperties > setProperties;
                if ( outArchive->QueryInterface( ::IID_ISetProperties,
                                                 reinterpret_cast< void** >( &setProperties ) ) != S_OK ) {
                    throw BitException( "ISetProperties unsupported" );
                }
                if ( setProperties->SetProperties( &names[ 0 ], &values[ 0 ],
                                                   static_cast< Int32 >( names.size() ) ) != S_OK ) {
                    throw BitException( "Cannot set properties of the archive" );
                }
            }
            return outArchive;
        }
    }
}

// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "../include/util.hpp"

#include <vector>

#include "Windows/COM.h"
#include "Windows/PropVariant.h"

#include "../include/bitexception.hpp"
#include "../include/opencallback.hpp"

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
                values.emplace_back( true );
            }
            if ( format.hasFeature( COMPRESSION_LEVEL ) ) {
                names.push_back( L"x" );
                values.emplace_back( static_cast< UInt32 >( compressionLevel ) );
            }
            if ( solidMode && format.hasFeature( SOLID_ARCHIVE ) ) {
                names.push_back( L"s" );
                values.emplace_back( solidMode );
            }

            if ( !names.empty() ) {
                CMyComPtr< ISetProperties > setProperties;
                if ( outArchive->QueryInterface( ::IID_ISetProperties,
                                                 reinterpret_cast< void** >( &setProperties ) ) != S_OK ) {
                    throw BitException( "ISetProperties unsupported" );
                }
                if ( setProperties->SetProperties( &names[ 0 ], &values[ 0 ],
                                                   static_cast< uint32_t >( names.size() ) ) != S_OK ) {
                    throw BitException( "Cannot set properties of the archive" );
                }
            }
            return outArchive;
        }

        // NOTE: this function is not a method of BitExtractor because it would dirty the header with extra dependencies
        CMyComPtr< IInArchive > openArchive( const Bit7zLibrary& lib, const BitInFormat& format,
                                             const wstring& in_file, const BitArchiveOpener& opener ) {
            CMyComPtr< IInArchive > inArchive;
            const GUID formatGUID = format.guid();
            lib.createArchiveObject( &formatGUID, &::IID_IInArchive, reinterpret_cast< void** >( &inArchive ) );

            auto* fileStreamSpec = new CInFileStream;
            CMyComPtr< IInStream > fileStream = fileStreamSpec;
            if ( !fileStreamSpec->Open( in_file.c_str() ) ) {
                throw BitException( L"Cannot open archive file '" + in_file + L"'" );
            }

            auto* openCallbackSpec = new OpenCallback( opener, in_file );

            CMyComPtr< IArchiveOpenCallback > openCallback( openCallbackSpec );
            if ( inArchive->Open( fileStream, nullptr, openCallback ) != S_OK ) {
                throw BitException( L"Cannot open archive '" + in_file + L"'" );
            }
            return inArchive;
        }
    }
}

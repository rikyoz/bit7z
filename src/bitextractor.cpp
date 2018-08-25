// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "../include/bitextractor.hpp"

#include "7zip/Archive/IArchive.h"
#include "Windows/COM.h"
#include "Windows/PropVariant.h"

#include "../include/bitpropvariant.hpp"
#include "../include/bitexception.hpp"
#include "../include/extractcallback.hpp"
#include "../include/memextractcallback.hpp"
#include "../include/fsutil.hpp"
#include "../include/util.hpp"

using namespace bit7z;
using namespace bit7z::filesystem;
using namespace bit7z::util;
using namespace NWindows;
using namespace NArchive;

using std::wstring;

BitExtractor::BitExtractor( const Bit7zLibrary& lib, const BitInFormat& format ) : BitArchiveOpener( lib, format ) {}

void BitExtractor::extract( const wstring& in_file, const wstring& out_dir ) const {
    extractItems( in_file, {}, out_dir );
}

//TODO: Improve reusing of code (both extract methods do the same operations when opening an archive)

/* Most of this code, though heavily modified, is taken from the main() of Client7z.cpp in the 7z SDK
 * Main changes made:
 *  + Generalized the code to work with any type of format (the original works only with 7z format)
 *  + Use of exceptions instead of error codes */
void BitExtractor::extractMatching( const wstring& in_file, const wstring& item_filter, const wstring& out_dir ) const {
    CMyComPtr< IInArchive > inArchive = openArchive( mLibrary, mFormat, in_file, *this );

    vector<uint32_t> matched_indices;
    // TODO: Use BitArchiveReader here
    if ( !item_filter.empty() ) {
        //Searching for files inside the archive that match the given filter
        uint32_t items_count;
        HRESULT result = inArchive->GetNumberOfItems( &items_count );
        if ( result == S_OK ) {
            for ( uint32_t index = 0; index < items_count; ++index ) {
                BitPropVariant propvar;
                result = inArchive->GetProperty( index, kpidPath, &propvar );
                if ( result == S_OK && !propvar.isEmpty() && propvar.type() == BitPropVariantType::String &&
                        fsutil::wildcard_match( item_filter, propvar.getString() ) ) {
                    matched_indices.push_back( index );
                }
            }
        }
    }

    extractToFileSystem( inArchive, in_file, out_dir, matched_indices );
}

void BitExtractor::extractItems( const wstring& in_file, const vector<uint32_t> indices, const wstring& out_dir ) const {
    CMyComPtr< IInArchive > inArchive = openArchive( mLibrary, mFormat, in_file, *this );
    extractToFileSystem( inArchive, in_file, out_dir, indices );
}

void BitExtractor::extract( const wstring& in_file, vector< byte_t >& out_buffer, unsigned int index ) {
    CMyComPtr< IInArchive > inArchive = openArchive( mLibrary, mFormat, in_file, *this );

    auto* extractCallbackSpec = new MemExtractCallback( *this, inArchive, out_buffer );

    const uint32_t indices[] = { index };

    CMyComPtr< IArchiveExtractCallback > extractCallback( extractCallbackSpec );
    if ( inArchive->Extract( indices, 1, NExtract::NAskMode::kExtract, extractCallback ) != S_OK ) {
        throw BitException( extractCallbackSpec->getErrorMessage() );
    }
}

void BitExtractor::test( const wstring& in_file ) {
    CMyComPtr< IInArchive > inArchive = openArchive( mLibrary, mFormat, in_file, *this );

    auto* extractCallbackSpec = new ExtractCallback( *this, inArchive, in_file, L"" );

    CMyComPtr< IArchiveExtractCallback > extractCallback( extractCallbackSpec );
    HRESULT res = inArchive->Extract( nullptr, static_cast< uint32_t >( -1 ), NExtract::NAskMode::kTest, extractCallback );
    if ( res != S_OK ) {
        throw BitException( extractCallbackSpec->getErrorMessage() + L" (error code: " + std::to_wstring(res) + L")" );
    }
}


void BitExtractor::extractToFileSystem( IInArchive* in_archive, const wstring& in_file,
                                        const wstring& out_dir, const vector<uint32_t>& indices ) const {

    //pointer to an array of the indices of the files to be extracted
    const uint32_t* item_indices = indices.empty() ? nullptr : indices.data();
    uint32_t num_items = indices.empty() ? static_cast< uint32_t >( -1 ) :
                         static_cast< uint32_t >( indices.size() );

    auto* extractCallbackSpec = new ExtractCallback( *this, in_archive, in_file, out_dir );

    CMyComPtr< IArchiveExtractCallback > extractCallback( extractCallbackSpec );
    HRESULT res = in_archive->Extract( item_indices, num_items, NExtract::NAskMode::kExtract, extractCallback );
    if ( res != S_OK ) {
        throw BitException( extractCallbackSpec->getErrorMessage() + L" (error code: " + std::to_wstring(res) + L")" );
    }
}

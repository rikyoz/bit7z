// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip DLLs.
 * Copyright (c) 2014-2018  Riccardo Ostani - All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * Bit7z is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with bit7z; if not, see https://www.gnu.org/licenses/.
 */

#include "../include/bitmemcompressor.hpp"

#include "7zip/Archive/IArchive.h"
#include "7zip/Common/FileStreams.h"
#include "7zip/Common/StreamObjects.h"

#include "../include/fsutil.hpp"
#include "../include/bitinputarchive.hpp"
#include "../include/bitexception.hpp"
#include "../include/coutmemstream.hpp"
#include "../include/coutmultivolstream.hpp"
#include "../include/memupdatecallback.hpp"

using namespace bit7z;
using namespace bit7z::filesystem;
using std::wstring;
using std::vector;

void compressOut( const BitArchiveCreator& creator,
                  const CMyComPtr< IOutArchive >& out_arc,
                  ISequentialOutStream* out_stream,
                  const vector< byte_t >& in_buffer,
                  const wstring& in_buffer_name,
                  const BitInputArchive* old_arc ) {
    auto* update_callback_spec = new MemUpdateCallback( creator, in_buffer, in_buffer_name, old_arc );
    uint32_t items_count = update_callback_spec->getItemsCount(); //old items count + new items count

    CMyComPtr< IArchiveUpdateCallback > update_callback( update_callback_spec );
    HRESULT result = out_arc->UpdateItems( out_stream, items_count, update_callback );

    if ( result == E_NOTIMPL ) {
        throw BitException( "Unsupported operation!" );
    }

    if ( result == E_FAIL && update_callback_spec->getErrorMessage().empty() ) {
        throw BitException( "Failed operation (unkwown error)!" );
    }

    if ( result != S_OK ) {
        throw BitException( update_callback_spec->getErrorMessage() );
    }
}

BitMemCompressor::BitMemCompressor( const Bit7zLibrary& lib, const BitInOutFormat& format )
    : BitArchiveCreator( lib, format ) {}

void BitMemCompressor::compress( const vector< byte_t >& in_buffer,
                                 const wstring& out_archive,
                                 wstring in_buffer_name ) const {

    if ( in_buffer_name.empty() ) {
        in_buffer_name = fsutil::filename( out_archive );
    }

    BitInputArchive* old_arc = nullptr;
    CMyComPtr< IOutArchive > new_arc = initOutArchive();
    CMyComPtr< IOutStream > out_file_stream;
    if ( mVolumeSize > 0 ) {
        auto* out_multivol_stream_spec = new COutMultiVolStream( mVolumeSize, out_archive );
        out_file_stream = out_multivol_stream_spec;
    } else {
        auto* out_file_stream_spec = new COutFileStream();
        out_file_stream = out_file_stream_spec;
        if ( !out_file_stream_spec->Create( out_archive.c_str(), false ) ) {
            if ( ::GetLastError() != ERROR_FILE_EXISTS ) { //unknown error
                throw BitException( L"Cannot create output archive file '" + out_archive + L"'" );
                // (error code: " + to_wstring( ::GetLastError() ) + L")" );
            }
            if ( !updateMode() ) { //output archive file already exists and no update mode set
                throw BitException( L"Cannot update existing archive file '" + out_archive + L"'" );
            }
            if ( !mFormat.hasFeature( FormatFeatures::MULTIPLE_FILES ) ) {
                //update mode is set but format does not support adding more files
                throw BitException( "Format does not support updating existing archive files" );
            }
            if ( !out_file_stream_spec->Create( ( out_archive + L".tmp" ).c_str(), false ) ) {
                //could not create temporary file
                throw BitException( L"Cannot create temp archive file for updating '" + out_archive + L"'" );
            }
            old_arc = new BitInputArchive( *this, out_archive );
            old_arc->initUpdatableArchive( &new_arc );
            setArchiveProperties( new_arc );
            //compressOut( *this, new_arc, out_file_stream, in_buffer, in_buffer_name, old_arc );
        }
    }
    compressOut( *this, new_arc, out_file_stream, in_buffer, in_buffer_name, old_arc );
    if ( old_arc ) {
        old_arc->close();
        dynamic_cast< COutFileStream* >( *&out_file_stream )->Close();
        delete old_arc;
        //remove old file and rename tmp file (move file with overwriting)
        bool renamed = fsutil::renameFile( out_archive + L".tmp", out_archive );
        if ( !renamed ) {
            throw BitException( L"Cannot rename temp archive file to  '" + out_archive + L"'" );
        }
        return;
    }
}

void BitMemCompressor::compress( const vector< byte_t >& in_buffer,
                                 vector< byte_t >& out_buffer,
                                 const wstring& in_buffer_name ) const {
    if ( !mFormat.hasFeature( INMEM_COMPRESSION ) ) {
        throw BitException( "Unsupported format for in-memory compression!" );
    }

    if ( !out_buffer.empty() ) {
        throw BitException( "Cannot overwrite or update a non empty buffer" );
    }

    CMyComPtr< IOutArchive > out_arc = initOutArchive();

    auto* out_mem_stream_spec = new COutMemStream( out_buffer );
    CMyComPtr< ISequentialOutStream > out_mem_stream( out_mem_stream_spec );

    compressOut( *this, out_arc, out_mem_stream, in_buffer, in_buffer_name, nullptr );
}

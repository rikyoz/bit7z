// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip DLLs.
 * Copyright (c) 2014-2019  Riccardo Ostani - All Rights Reserved.
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

#include "include/outputarchive.hpp"

#include "../include/coutmultivolstream.hpp"
#include "../include/coutmemstream.hpp"
#include "../include/updatecallback.hpp"
#include "../include/memupdatecallback.hpp"
#include "../include/fsutil.hpp"

using namespace bit7z;

OutputArchive::OutputArchive( const BitArchiveCreator& creator ) : mCreator( creator ) {
    const GUID format_GUID = mCreator.format().guid();
    mCreator.library().createArchiveObject( &format_GUID,
                                            &::IID_IOutArchive,
                                            reinterpret_cast< void** >( &mOutArc ) );
    mCreator.setArchiveProperties( mOutArc );
}

void OutputArchive::compress( const vector< FSItem >& in_items, const wstring& out_archive ) {
    unique_ptr< BitInputArchive > old_arc = nullptr;
    CMyComPtr< IOutStream > out_file_stream = initOutFileStream( out_archive, old_arc );

    CMyComPtr< CompressCallback > update_callback = new UpdateCallback( mCreator, in_items, old_arc.get() );
    compress( out_file_stream, update_callback );

    cleanupOldArc( old_arc.get(), out_file_stream, out_archive );
}

void OutputArchive::compress( const vector< byte_t >& in_buffer,
                              const wstring& in_buffer_name,
                              const wstring& out_archive ) {
    unique_ptr< BitInputArchive > old_arc = nullptr;
    CMyComPtr< IOutStream > out_file_stream = initOutFileStream( out_archive, old_arc );

    CMyComPtr< CompressCallback > update_callback = new MemUpdateCallback( mCreator, in_buffer, in_buffer_name, old_arc.get() );
    compress( out_file_stream, update_callback );

    cleanupOldArc( old_arc.get(), out_file_stream, out_archive );
}

void OutputArchive::compress( const vector< FSItem >& in_items, vector< byte_t >& out_buffer ) {
    CMyComPtr< ISequentialOutStream > out_mem_stream = new COutMemStream( out_buffer );
    CMyComPtr< CompressCallback > update_callback = new UpdateCallback( mCreator, in_items );
    compress( out_mem_stream, update_callback );
}

void OutputArchive::compress( const vector< byte_t >& in_buffer, const wstring& in_buffer_name, vector< byte_t >& out_buffer ) {
    CMyComPtr< ISequentialOutStream > out_mem_stream = new COutMemStream( out_buffer );
    CMyComPtr< CompressCallback > update_callback = new MemUpdateCallback( mCreator, in_buffer, in_buffer_name );
    compress( out_mem_stream, update_callback );
}

CMyComPtr< IOutStream > OutputArchive::initOutFileStream( const wstring& out_archive, unique_ptr< BitInputArchive >& old_arc ) {
    CMyComPtr< IOutStream > out_file_stream;
    if ( mCreator.volumeSize() > 0 ) {
        out_file_stream = new COutMultiVolStream( mCreator.volumeSize(), out_archive );
    } else {
        auto* out_file_stream_spec = new COutFileStream();
        //NOTE: if any exception occurs in the following ifs, the file stream obj is released thanks to the CMyComPtr
        out_file_stream = out_file_stream_spec;
        if ( !out_file_stream_spec->Create( out_archive.c_str(), false ) ) {
            if ( ::GetLastError() != ERROR_FILE_EXISTS ) { //unknown error
                throw BitException( L"Cannot create output archive file '" + out_archive + L"'" );
            }
            if ( !mCreator.updateMode() ) { //output archive file already exists and no update mode set
                throw BitException( L"Cannot update existing archive file '" + out_archive + L"'" );
            }
            if ( !mCreator.compressionFormat().hasFeature( FormatFeatures::MULTIPLE_FILES ) ) {
                //update mode is set but format does not support adding more files
                throw BitException( "Format does not support updating existing archive files" );
            }
            if ( !out_file_stream_spec->Create( ( out_archive + L".tmp" ).c_str(), false ) ) {
                //could not create temporary file
                throw BitException( L"Cannot create temp archive file for updating '" + out_archive + L"'" );
            }
            old_arc = std::make_unique< BitInputArchive >( mCreator, out_archive );
            old_arc->initUpdatableArchive( &mOutArc );
            mCreator.setArchiveProperties( mOutArc );
        }
    }
    return out_file_stream;
}

HRESULT OutputArchive::compress( ISequentialOutStream* out_stream, CompressCallback* update_callback ) {
    HRESULT result = mOutArc->UpdateItems( out_stream, update_callback->itemsCount(), update_callback );

    if ( result == E_NOTIMPL ) {
        throw BitException( "Unsupported operation!" );
    }

    if ( result == E_FAIL && update_callback->getErrorMessage().empty() ) {
        throw BitException( "Failed operation (unkwown error)!" );
    }

    if ( result != S_OK ) {
        throw BitException( update_callback->getErrorMessage() );
    }

    return result;
}

void OutputArchive::cleanupOldArc( BitInputArchive* old_arc, IOutStream* out_stream, const wstring& out_archive ) {
    if ( old_arc ) {
        old_arc->close();
		auto out_file_stream = dynamic_cast<COutFileStream*>(&*out_stream); //cast should not fail, but anyway...
		if ( out_file_stream ) {
			out_file_stream->Close();
		}
        //remove old file and rename tmp file (move file with overwriting)
        bool renamed = fsutil::renameFile( out_archive + L".tmp", out_archive );
        if ( !renamed ) {
            throw BitException( L"Cannot rename temp archive file to  '" + out_archive + L"'" );
        }
    }
}

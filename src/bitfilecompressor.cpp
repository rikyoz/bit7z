// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "bitfilecompressor.hpp"

#include "biterror.hpp"
#include "bitexception.hpp"
#include "bitoutputarchive.hpp"

using namespace std;
using namespace bit7z;

BitFileCompressor::BitFileCompressor( const Bit7zLibrary& lib, const BitInOutFormat& format )
    : BitCompressor( lib, format ) {}

/* from filesystem to filesystem */

void BitFileCompressor::compress( const vector< tstring >& in_paths, const tstring& out_file ) const {
    if ( in_paths.size() > 1 && !compressionFormat().hasFeature( FormatFeatures::MultipleFiles ) ) {
        throw BitException( "Cannot compress multiple files", make_error_code( BitError::UnsupportedOperation ) );
    }
    BitOutputArchive output_archive{ *this, out_file };
    output_archive.addItems( in_paths );
    output_archive.compressTo( out_file );
}

void BitFileCompressor::compress( const map< tstring, tstring >& in_paths, const tstring& out_file ) const {
    if ( in_paths.size() > 1 && !compressionFormat().hasFeature( FormatFeatures::MultipleFiles ) ) {
        throw BitException( "Cannot compress multiple files", make_error_code( BitError::UnsupportedOperation ) );
    }
    BitOutputArchive output_archive{ *this, out_file };
    output_archive.addItems( in_paths );
    output_archive.compressTo( out_file );
}

void BitFileCompressor::compressFiles( const vector< tstring >& in_files, const tstring& out_file ) const {
    if ( in_files.size() > 1 && !compressionFormat().hasFeature( FormatFeatures::MultipleFiles ) ) {
        throw BitException( "Cannot compress multiple files", make_error_code( BitError::UnsupportedOperation ) );
    }
    BitOutputArchive output_archive{ *this, out_file };
    output_archive.addFiles( in_files );
    output_archive.compressTo( out_file );
}

void BitFileCompressor::compressFiles( const tstring& in_dir, const tstring& out_file,
                                       bool recursive, const tstring& filter ) const {
    if ( !compressionFormat().hasFeature( FormatFeatures::MultipleFiles ) ) {
        throw BitException( "Cannot compress multiple files", make_error_code( BitError::UnsupportedOperation ) );
    }
    BitOutputArchive output_archive{ *this, out_file };
    output_archive.addFiles( in_dir, filter, recursive );
    output_archive.compressTo( out_file );
}

void BitFileCompressor::compressDirectory( const tstring& in_dir, const tstring& out_file ) const {
    compressFiles( in_dir, out_file, true, tstring{} );
}

/* from filesystem to stream */

void BitFileCompressor::compress( const vector< tstring >& in_paths, ostream& out_stream ) const {
    if ( in_paths.size() > 1 && !compressionFormat().hasFeature( FormatFeatures::MultipleFiles ) ) {
        throw BitException( "Cannot compress multiple files", make_error_code( BitError::UnsupportedOperation ) );
    }
    BitOutputArchive output_archive{ *this };
    output_archive.addItems( in_paths );
    output_archive.compressTo( out_stream );
}

void BitFileCompressor::compress( const map< tstring, tstring >& in_paths, ostream& out_stream ) const {
    if ( in_paths.size() > 1 && !compressionFormat().hasFeature( FormatFeatures::MultipleFiles ) ) {
        throw BitException( "Cannot compress multiple files", make_error_code( BitError::UnsupportedOperation ) );
    }
    BitOutputArchive output_archive{ *this };
    output_archive.addItems( in_paths );
    output_archive.compressTo( out_stream );
}

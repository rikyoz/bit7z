// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2023 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <algorithm> // for std::all_of

#include "bitabstractarchivecreator.hpp"
#include "biterror.hpp"
#include "bitexception.hpp"
#include "internal/archiveproperties.hpp"

using namespace bit7z;

auto isValidCompressionMethod( const BitInOutFormat& format, BitCompressionMethod method ) noexcept -> bool {
    switch ( method ) {
        case BitCompressionMethod::Copy:
            return format == BitFormat::SevenZip || format == BitFormat::Zip || format == BitFormat::Tar ||
                   format == BitFormat::Wim;
        case BitCompressionMethod::Ppmd:
        case BitCompressionMethod::Lzma:
            return format == BitFormat::SevenZip || format == BitFormat::Zip;
        case BitCompressionMethod::Lzma2:
            return format == BitFormat::SevenZip || format == BitFormat::Xz;
        case BitCompressionMethod::BZip2:
            return format == BitFormat::SevenZip || format == BitFormat::BZip2 || format == BitFormat::Zip;
        case BitCompressionMethod::Deflate:
            return format == BitFormat::GZip || format == BitFormat::Zip;
        case BitCompressionMethod::Deflate64:
            return format == BitFormat::Zip;
        default:
            return false;
    }
}

auto isValidDictionarySize( BitCompressionMethod method, uint32_t dictionary_size ) noexcept -> bool {
    static constexpr auto MAX_LZMA_DICTIONARY_SIZE = 1536 * ( 1LL << 20 ); // less than 1536 MiB
    static constexpr auto MAX_PPMD_DICTIONARY_SIZE = ( 1LL << 30 );        // less than 1 GiB, i.e., 2^30 bytes
    static constexpr auto MAX_BZIP2_DICTIONARY_SIZE = 900 * ( 1LL << 10 ); // less than 900 KiB

    switch ( method ) {
        case BitCompressionMethod::Lzma:
        case BitCompressionMethod::Lzma2:
            return dictionary_size <= MAX_LZMA_DICTIONARY_SIZE;
        case BitCompressionMethod::Ppmd:
            return dictionary_size <= MAX_PPMD_DICTIONARY_SIZE;
        case BitCompressionMethod::BZip2:
            return dictionary_size <= MAX_BZIP2_DICTIONARY_SIZE;
        default:
            return false;
    }
}

auto isValidWordSize( const BitInOutFormat& format, BitCompressionMethod method, uint32_t word_size ) noexcept -> bool {
    static constexpr auto MIN_LZMA_WORD_SIZE = 5u;
    static constexpr auto MAX_LZMA_WORD_SIZE = 273u;
    static constexpr auto MIN_PPMD_WORD_SIZE = 2u;
    static constexpr auto MAX_ZIP_PPMD_WORD_SIZE = 16u;
    static constexpr auto MAX_7Z_PPMD_WORD_SIZE = 32u;
    static constexpr auto MIN_DEFLATE_WORD_SIZE = 3u;
    static constexpr auto MAX_DEFLATE_WORD_SIZE = 258u;
    static constexpr auto MAX_DEFLATE64_WORD_SIZE = MAX_DEFLATE_WORD_SIZE - 1;

    if ( word_size == 0 ) {
        return true; // reset to default value
    }

    switch ( method ) {
        case BitCompressionMethod::Lzma:
        case BitCompressionMethod::Lzma2:
            return word_size >= MIN_LZMA_WORD_SIZE && word_size <= MAX_LZMA_WORD_SIZE;
        case BitCompressionMethod::Ppmd:
            return word_size >= MIN_PPMD_WORD_SIZE && word_size <=
                                                      ( format == BitFormat::Zip ? MAX_ZIP_PPMD_WORD_SIZE
                                                                                 : MAX_7Z_PPMD_WORD_SIZE );
        case BitCompressionMethod::Deflate64:
            return word_size >= MIN_DEFLATE_WORD_SIZE && word_size <= MAX_DEFLATE64_WORD_SIZE;
        case BitCompressionMethod::Deflate:
            return word_size >= MIN_DEFLATE_WORD_SIZE && word_size <= MAX_DEFLATE_WORD_SIZE;
        default:
            return false;
    }
}

auto methodName( BitCompressionMethod method ) noexcept -> const wchar_t* {
    switch ( method ) {
        case BitCompressionMethod::Copy:
            return L"Copy";
        case BitCompressionMethod::Ppmd:
            return L"PPMd";
        case BitCompressionMethod::Lzma:
            return L"LZMA";
        case BitCompressionMethod::Lzma2:
            return L"LZMA2";
        case BitCompressionMethod::BZip2:
            return L"BZip2";
        case BitCompressionMethod::Deflate:
            return L"Deflate";
        case BitCompressionMethod::Deflate64:
            return L"Deflate64";
        default:
            return L"Unknown"; //this should not happen!
    }
}

BitAbstractArchiveCreator::BitAbstractArchiveCreator( const Bit7zLibrary& lib,
                                                      const BitInOutFormat& format,
                                                      tstring password,
                                                      UpdateMode update_mode )
    : BitAbstractArchiveHandler( lib, std::move( password ) ),
      mFormat( format ),
      mUpdateMode( update_mode ),
      mCompressionLevel( BitCompressionLevel::Normal ),
      mCompressionMethod( format.defaultMethod() ),
      mDictionarySize( 0 ),
      mWordSize( 0 ),
      mCryptHeaders( false ),
      mSolidMode( false ),
      mVolumeSize( 0 ),
      mThreadsCount( 0 ) {
    setRetainDirectories( false );
}

auto BitAbstractArchiveCreator::format() const noexcept -> const BitInFormat& {
    return mFormat;
}

auto BitAbstractArchiveCreator::compressionFormat() const noexcept -> const BitInOutFormat& {
    return mFormat;
}

auto BitAbstractArchiveCreator::cryptHeaders() const noexcept -> bool {
    return mCryptHeaders;
}

auto BitAbstractArchiveCreator::compressionLevel() const noexcept -> BitCompressionLevel {
    return mCompressionLevel;
}

auto BitAbstractArchiveCreator::compressionMethod() const noexcept -> BitCompressionMethod {
    return mCompressionMethod;
}

auto BitAbstractArchiveCreator::dictionarySize() const noexcept -> uint32_t {
    return mDictionarySize;
}

auto BitAbstractArchiveCreator::wordSize() const noexcept -> uint32_t {
    return mWordSize;
}

auto BitAbstractArchiveCreator::solidMode() const noexcept -> bool {
    return mSolidMode;
}

auto BitAbstractArchiveCreator::updateMode() const noexcept -> UpdateMode {
    return mUpdateMode;
}

auto BitAbstractArchiveCreator::volumeSize() const noexcept -> uint64_t {
    return mVolumeSize;
}

auto BitAbstractArchiveCreator::threadsCount() const noexcept -> uint32_t {
    return mThreadsCount;
}

void BitAbstractArchiveCreator::setPassword( const tstring& password ) {
    setPassword( password, mCryptHeaders );
}

#ifndef BIT7Z_DISABLE_ZIP_ASCII_PWD_CHECK
auto is_ascii( const tstring& str ) -> bool {
    return std::all_of( str.begin(), str.end(), []( tchar character ) -> bool {
        // Note: 7-zip supports the DEL character (code 127), while bit7z doesn't.
        constexpr auto first_ascii_char = 32; // A
        constexpr auto last_ascii_char = 127;
        return character >= first_ascii_char && character < last_ascii_char;
    } );
}
#endif

void BitAbstractArchiveCreator::setPassword( const tstring& password, bool crypt_headers ) {
#ifndef BIT7Z_DISABLE_ZIP_ASCII_PWD_CHECK
    if ( mFormat == BitFormat::Zip && !is_ascii( password ) ) {
        throw BitException( "Invalid password", make_error_code( BitError::InvalidZipPassword ) );
    }
#endif
    BitAbstractArchiveHandler::setPassword( password );
    mCryptHeaders = ( password.length() > 0 ) && crypt_headers;
}

void BitAbstractArchiveCreator::setCompressionLevel( BitCompressionLevel level ) noexcept {
    mCompressionLevel = level;
    mDictionarySize = 0; //reset dictionary size to default for the compression level
    mWordSize = 0; //reset word size to default for the compression level
}

void BitAbstractArchiveCreator::setCompressionMethod( BitCompressionMethod method ) {
    if ( !isValidCompressionMethod( mFormat, method ) ) {
        throw BitException( "Cannot set the compression method",
                            make_error_code( BitError::InvalidCompressionMethod ) );
    }
    if ( mFormat.hasFeature( FormatFeatures::MultipleMethods ) ) {
        /* even though the compression method is valid, we set it only if the format supports
         * different methods than the default one (i.e., setting BitCompressionMethod::BZip2
         * of a BitFormat::BZip2 archive does nothing!) */
        mCompressionMethod = method;
        mDictionarySize = 0; //reset dictionary size to default value for the method
        mWordSize = 0; //reset word size to default value for the method
    }
}

void BitAbstractArchiveCreator::setDictionarySize( uint32_t dictionary_size ) {
    if ( mCompressionMethod == BitCompressionMethod::Copy ||
         mCompressionMethod == BitCompressionMethod::Deflate ||
         mCompressionMethod == BitCompressionMethod::Deflate64 ) {
        //ignoring setting dictionary size for copy method and for methods having fixed dictionary size (deflate family)
        return;
    }
    if ( !isValidDictionarySize( mCompressionMethod, dictionary_size ) ) {
        throw BitException( "Cannot set the dictionary size", make_error_code( BitError::InvalidDictionarySize ) );
    }
    mDictionarySize = dictionary_size;
}

void BitAbstractArchiveCreator::setWordSize( uint32_t word_size ) {
    if ( mCompressionMethod == BitCompressionMethod::Copy || mCompressionMethod == BitCompressionMethod::BZip2 ) {
        return;
    }
    if ( !isValidWordSize( mFormat, mCompressionMethod, word_size ) ) {
        throw BitException( "Cannot set the word size", make_error_code( BitError::InvalidWordSize ) );
    }
    mWordSize = word_size;
}

void BitAbstractArchiveCreator::setSolidMode( bool solid_mode ) noexcept {
    mSolidMode = solid_mode;
}

void BitAbstractArchiveCreator::setUpdateMode( UpdateMode mode ) {
    mUpdateMode = mode;
}

void BitAbstractArchiveCreator::setUpdateMode( bool can_update ) {
    // Same behavior as in bit7z v3 API.
    setUpdateMode( can_update ? UpdateMode::Append : UpdateMode::None );
}

void BitAbstractArchiveCreator::setVolumeSize( uint64_t volume_size ) noexcept {
    mVolumeSize = volume_size;
}

void BitAbstractArchiveCreator::setThreadsCount( uint32_t threads_count ) noexcept {
    mThreadsCount = threads_count;
}

auto dictionaryPropertyName( const BitInOutFormat& format, BitCompressionMethod method ) -> const wchar_t* {
    if ( format == BitFormat::SevenZip ) {
        return ( method == BitCompressionMethod::Ppmd ? L"0mem" : L"0d" );
    }
    return ( method == BitCompressionMethod::Ppmd ? L"mem" : L"d" );
}

auto wordSizePropertyName( const BitInOutFormat& format, BitCompressionMethod method ) -> const wchar_t* {
    if ( format == BitFormat::SevenZip ) {
        return ( method == BitCompressionMethod::Ppmd ? L"0o" : L"0fb" );
    }
    return ( method == BitCompressionMethod::Ppmd ? L"o" : L"fb" );
}

auto BitAbstractArchiveCreator::archiveProperties() const -> ArchiveProperties {
    ArchiveProperties properties = {};
    if ( mCryptHeaders && mFormat.hasFeature( FormatFeatures::HeaderEncryption ) ) {
        properties.setProperty( L"he", true );
    }
    if ( mFormat.hasFeature( FormatFeatures::CompressionLevel ) ) {
        properties.setProperty( L"x", static_cast< uint32_t >( mCompressionLevel ) );

        if ( mFormat.hasFeature( FormatFeatures::MultipleMethods ) && mCompressionMethod != mFormat.defaultMethod() ) {
            const auto* property_name = ( mFormat == BitFormat::SevenZip ) ? L"0" : L"m";
            properties.setProperty( property_name, methodName( mCompressionMethod ) );
        }
    }
    if ( mFormat.hasFeature( FormatFeatures::SolidArchive ) ) {
        properties.setProperty( L"s", mSolidMode );
#ifndef _WIN32
        if ( mSolidMode ) {
            /* NOTE: Apparently, p7zip requires the filters to be set off for the solid compression to work.
               The strangest thing is... in my tests this happens only in WSL!
               I've tested the same code on a Linux VM, and it works without disabling the filters! */
            // TODO: So, for now I disable them, but this will need further investigation!
            properties.setProperty( L"f", false );
        }
#endif
    }
    if ( mThreadsCount != 0 ) {
        properties.setProperty( L"mt", mThreadsCount );
    }
    if ( mDictionarySize != 0 ) {
        properties.setProperty( dictionaryPropertyName( mFormat, mCompressionMethod ),
                                std::to_wstring( mDictionarySize ) + L"b" );
    }
    if ( mWordSize != 0 ) {
        properties.setProperty( wordSizePropertyName( mFormat, mCompressionMethod ), mWordSize );
    }
    properties.addProperties( mExtraProperties );
    return properties;
}

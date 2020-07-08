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

#include "../include/fileupdatecallback.hpp"

#include "../include/cfileinstream.hpp"
#include "../include/cfileoutstream.hpp"

using namespace bit7z;

/* Most of this code is taken from the CUpdateCallback class in Client7z.cpp of the 7z SDK
 * Main changes made:
 *  + Use of std::vector instead of CRecordVector, CObjectVector and UStringVector
 *  + Use of tstring instead of UString (see Callback base interface)
 *  + Error messages are not showed (see comments in ExtractCallback)
 *  + The work performed originally by the Init method is now performed by the class constructor
 *  + FSItem class is used instead of CDirItem struct */

FileUpdateCallback::FileUpdateCallback( const BitArchiveCreator& creator,
                                        const vector< FSItem >& new_items )
    : UpdateCallback( creator ),
      mNewItems( new_items ),
      mVolSize( 0 ) {}

HRESULT FileUpdateCallback::GetProperty( UInt32 index, PROPID propID, PROPVARIANT* value ) {
    BitPropVariant prop;
    if ( propID == kpidIsAnti ) {
        prop = false;
    } else if ( index < mOldArcItemsCount ) {
        prop = mOldArc->getItemProperty( index, static_cast< BitProperty >( propID ) );
    } else {
        const FSItem& new_item = mNewItems[ static_cast< size_t >( index - mOldArcItemsCount ) ];
        switch ( propID ) {
            case kpidPath:
                prop = new_item.inArchivePath().wstring();
                break;
            case kpidIsDir:
                prop = new_item.isDir();
                break;
            case kpidSize:
                prop = new_item.size();
                break;
            case kpidAttrib:
                prop = new_item.attributes();
                break;
            case kpidCTime:
                prop = new_item.creationTime();
                break;
            case kpidATime:
                prop = new_item.lastAccessTime();
                break;
            case kpidMTime:
                prop = new_item.lastWriteTime();
                break;
            default: //empty prop
                break;
        }
    }

    *value = prop;
    prop.bstrVal = nullptr;
    return S_OK;
}

uint32_t FileUpdateCallback::itemsCount() const {
    return mOldArcItemsCount + static_cast< uint32_t >( mNewItems.size() );
}

HRESULT FileUpdateCallback::GetStream( UInt32 index, ISequentialInStream** inStream ) {
    RINOK( Finalize() )

    if ( index < mOldArcItemsCount ) { //old item in the archive
        return S_OK;
    }

    const FSItem& new_item = mNewItems[ static_cast< size_t >( index - mOldArcItemsCount ) ];

    if ( mHandler.fileCallback() ) {
        mHandler.fileCallback()( new_item.name() );
    }

    if ( new_item.isDir() ) {
        return S_OK;
    }

    auto path = new_item.path();
    CMyComPtr< CFileInStream > inStreamLoc = new CFileInStream( path );

    if ( inStreamLoc->fail() ) {
        std::error_code ec;
        if ( fs::exists( path, ec ) ) {
            ec = std::make_error_code( std::errc::file_exists );
        }
        mFailedFiles.emplace_back( path.native(), ec );
        return S_FALSE;
    }

    *inStream = inStreamLoc.Detach();
    return S_OK;
}

HRESULT FileUpdateCallback::GetVolumeSize( UInt32 /*index*/, UInt64* size ) {
    if ( mVolSize == 0 ) { return S_FALSE; }

    *size = mVolSize;
    return S_OK;
}

HRESULT FileUpdateCallback::GetVolumeStream( UInt32 index, ISequentialOutStream** volumeStream ) {
    tstring res = to_tstring( index + 1 );
    if ( res.length() < 3 ) {
        //adding leading zeros for a total res length of 3 (e.g. volume 42 will have extension .042)
        res.insert( res.begin(), 3 - res.length(), TSTRING( '0' ) );
    }

    tstring fileName = mVolName + TSTRING( '.' ) + res;// + mVolExt;
    CMyComPtr< CFileOutStream > stream = new CFileOutStream( fileName );

    if ( stream->fail() ) {
        return HRESULT_FROM_WIN32( ERROR_OPEN_FAILED );
    }

    *volumeStream = stream.Detach();
    return S_OK;
}

void FileUpdateCallback::throwException( HRESULT error ) {
    if ( !mFailedFiles.empty() ) {
        throw BitException( "Error compressing files", make_hresult_code( error ), std::move( mFailedFiles ) );
    }
    Callback::throwException( error );
}

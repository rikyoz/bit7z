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

//debug includes:
//#include <sstream>
//#include <iomanip>
//end of debug includes

#include "7zip/Common/FileStreams.h"

#include "../include/bitpropvariant.hpp"
#include "../include/fsutil.hpp"

#include <sstream>

using namespace bit7z;

/* Most of this code is taken from the CUpdateCallback class in Client7z.cpp of the 7z SDK
 * Main changes made:
 *  + Use of std::vector instead of CRecordVector, CObjectVector and UStringVector
 *  + Use of std::wstring instead of UString (see Callback base interface)
 *  + Error messages are not showed (see comments in ExtractCallback)
 *  + The work performed originally by the Init method is now performed by the class constructor
 *  + FSItem class is used instead of CDirItem struct */

FileUpdateCallback::FileUpdateCallback( const BitArchiveCreator& creator,
                                        const vector< FSItem >& new_items )
    : UpdateCallback( creator ),
      mNewItems( new_items ),
      mVolSize( 0 ) {}

FileUpdateCallback::~FileUpdateCallback() {}

// debug purposes
/*wstring to_string( FILETIME ftime ) {// ISO format, time zone designator Z == zero (UTC)
    SYSTEMTIME utc ;
    ::FileTimeToSystemTime( std::addressof(ftime), std::addressof(utc) );

    std::wostringstream stm;
    const auto w2 = std::setw(2);
    stm << std::setfill(L'0') << std::setw(4) << utc.wYear << L'-' << w2 << utc.wMonth
        << L'-' << w2 << utc.wDay << L' ' << w2 << utc.wYear << L' ' << w2 << utc.wHour
        << L':' << w2 << utc.wMinute << L':' << w2 << utc.wSecond << L'Z' ;
    return stm.str();
}*/

HRESULT FileUpdateCallback::GetProperty( UInt32 index, PROPID propID, PROPVARIANT* value ) {
    BitPropVariant prop;
    if ( propID == kpidIsAnti ) {
        prop = false;
    } else if ( index < mOldArcItemsCount ) {
        prop = mOldArc->getItemProperty( index, static_cast< BitProperty >( propID ) );
    } else {
        const FSItem& new_item = mNewItems[ index - mOldArcItemsCount ];
        switch ( propID ) {
            case kpidPath:
                prop = new_item.inArchivePath();
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
                /*wcout << L"dirItem " << dirItem.name()
                      << " last access time: " << to_string( dirItem.lastAccessTime() ) << endl;*/
                break;
            case kpidMTime:
                prop = new_item.lastWriteTime();
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
    RINOK( Finilize() );

    if ( index < mOldArcItemsCount ) { //old item in the archive
        return S_OK;
    }

    const FSItem& new_item = mNewItems[ index - mOldArcItemsCount ];

    if ( mHandler.fileCallback() ) {
        mHandler.fileCallback()( new_item.name() );
    }

    if ( new_item.isDir() ) {
        return S_OK;
    }

    auto* inStreamSpec = new CInFileStream;
    CMyComPtr< ISequentialInStream > inStreamLoc( inStreamSpec );
    wstring path = new_item.path();

    if ( !inStreamSpec->Open( path.c_str() ) ) {
        DWORD last_error = ::GetLastError();
        mFailedFiles.emplace_back( path, HRESULT_FROM_WIN32( last_error ) );
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
    wstring res = ( index < 9 ? L"00" : index < 99 ? L"0" : L"" ) + std::to_wstring( index + 1 );

    wstring fileName = mVolName + L'.' + res;// + mVolExt;
    auto* streamSpec = new COutFileStream;
    CMyComPtr< ISequentialOutStream > streamLoc( streamSpec );

    if ( !streamSpec->Create( fileName.c_str(), false ) ) {
        //return ::GetLastError();
        DWORD last_error = ::GetLastError();
        return ( last_error == 0 ) ? E_FAIL : HRESULT_FROM_WIN32( last_error );
    }

    *volumeStream = streamLoc.Detach();
    return S_OK;
}

wstring FileUpdateCallback::getErrorMessage() const {
    if ( !mFailedFiles.empty() ) {
        std::wstringstream wsstream;
        wsstream << L"Error for files: " << std::endl;
        for ( const auto& failed_file : mFailedFiles ) {
            wsstream << failed_file.first << L" (error code: " << failed_file.second << L")" << std::endl;
        }
        return wsstream.str();
    }
    return Callback::getErrorMessage();
}

// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip DLLs.
 * Copyright (c) 2014-2021  Riccardo Ostani - All Rights Reserved.
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

#include "internal/bufferitem.hpp"

#include "internal/cbufferinstream.hpp"

using bit7z::BufferItem;
using bit7z::BitPropVariant;
using bit7z::tstring;

BufferItem::BufferItem( const vector <byte_t>& buffer, const tstring& name ) : mBuffer{ buffer }, mBufferName{ name } {}

tstring BufferItem::name() const {
    return mBufferName.filename();
}

fs::path BufferItem::path() const {
    return mBufferName;
}

fs::path BufferItem::inArchivePath() const {
    return mBufferName;
}

BitPropVariant BufferItem::getProperty( BitProperty propID ) const {
    BitPropVariant prop;
    switch ( propID ) {
        case BitProperty::Path:
            prop = mBufferName.wstring();
            break;
        case BitProperty::IsDir:
            prop = false;
            break;
        case BitProperty::Size:
            prop = static_cast< uint64_t >( sizeof( byte_t ) * mBuffer.size() );
            break;
        case BitProperty::Attrib:
            prop = static_cast< uint32_t >( FILE_ATTRIBUTE_NORMAL );
            break;
        case BitProperty::CTime:
        case BitProperty::ATime:
        case BitProperty::MTime: {
            FILETIME ft;
            SYSTEMTIME st;

            GetSystemTime( &st ); // gets current time
            SystemTimeToFileTime( &st, &ft ); // converts to file time format
            prop = ft;
            break;
        }
        default: //empty prop
            break;
    }
    return prop;
}

HRESULT BufferItem::getStream( ISequentialInStream** inStream ) const {
    CMyComPtr< ISequentialInStream > inStreamLoc = new CBufferInStream( mBuffer );
    *inStream = inStreamLoc.Detach();
    return S_OK;
}

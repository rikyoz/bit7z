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

#include "internal/processeditem.hpp"

#include "bitexception.hpp"

using bit7z::ProcessedItem;

ProcessedItem::ProcessedItem()
    : mModifiedTime{}, mIsModifiedTimeDefined{ false }, mAttributes{ 0 }, mAreAttributesDefined{ false }
{}

void ProcessedItem::loadItemInfo( const BitInputArchive& input_archive, std::uint32_t item_index ) {
    loadFilePath( input_archive, item_index );
    loadAttributes( input_archive, item_index );
    loadModifiedTime( input_archive, item_index );
}

fs::path ProcessedItem::path() const {
    return mFilePath;
}

uint32_t ProcessedItem::attributes() const {
    return mAttributes;
}

FILETIME ProcessedItem::modifiedTime() const {
    return mModifiedTime;
}

void ProcessedItem::loadFilePath( const BitInputArchive& input_archive, uint32_t item_index ) {
    BitPropVariant prop = input_archive.itemProperty( item_index, BitProperty::Path );

    switch ( prop.type() ) {
        case BitPropVariantType::Empty:
            mFilePath = fs::path();
            break;

        case BitPropVariantType::String:
            mFilePath = fs::path( prop.getString() );
            break;

        default:
            throw BitException( "Could not load file path information of item", make_hresult_code( E_FAIL ) );
    }
}

void ProcessedItem::loadAttributes( const BitInputArchive& input_archive, uint32_t item_index ) {
    mAttributes = 0;
    mAreAttributesDefined = false;

    BitPropVariant posixAttributes = input_archive.itemProperty( item_index, BitProperty::PosixAttrib );
    switch ( posixAttributes.type() ) {
        case BitPropVariantType::Empty:
            break;

        case BitPropVariantType::UInt32:
            mAttributes = ( posixAttributes.getUInt32() << 16u ) | FILE_ATTRIBUTE_UNIX_EXTENSION;
            mAreAttributesDefined = true;
            break;

        default:
            throw BitException( "Could not load posix attributes of item", make_hresult_code( E_FAIL ) );
    }

    // Get Attrib
    BitPropVariant attributes = input_archive.itemProperty( item_index, BitProperty::Attrib );
    switch ( attributes.type() ) {
        case BitPropVariantType::Empty:
            break;

        case BitPropVariantType::UInt32:
            mAttributes = attributes.getUInt32();
            mAreAttributesDefined = true;
            break;

        default:
            throw BitException( "Could not load attributes of item", make_hresult_code( E_FAIL ) );
    }
}

void ProcessedItem::loadModifiedTime( const BitInputArchive& input_archive, uint32_t item_index ) {
    BitPropVariant modifiedTime = input_archive.itemProperty( item_index, BitProperty::MTime );

    switch ( modifiedTime.type() ) {
        case BitPropVariantType::Empty:
            mIsModifiedTimeDefined = false;
            break;

        case BitPropVariantType::FileTime:
            mModifiedTime = modifiedTime.getFileTime();
            mIsModifiedTimeDefined = true;
            break;

        default:
            throw BitException( "Could not load last modified time of item", make_hresult_code( E_FAIL ) );
    }
}

bool bit7z::ProcessedItem::areAttributesDefined() const {
    return mAreAttributesDefined;
}

bool bit7z::ProcessedItem::isModifiedTimeDefined() const {
    return mIsModifiedTimeDefined;
}

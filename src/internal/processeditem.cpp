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

#include "internal/processeditem.hpp"

#include "bitexception.hpp"

using bit7z::BitInputArchive;
using bit7z::ProcessedItem;

ProcessedItem::ProcessedItem()
    : mModifiedTime{}, mIsModifiedTimeDefined{ false }, mAttributes{ 0 }, mAreAttributesDefined{ false } {}

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
    const BitPropVariant prop = input_archive.itemProperty( item_index, BitProperty::Path );

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

    // Get posix attributes
    const BitPropVariant posixAttributes = input_archive.itemProperty( item_index, BitProperty::PosixAttrib );
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

    // Get attributes
    const BitPropVariant attributes = input_archive.itemProperty( item_index, BitProperty::Attrib );
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
    const BitPropVariant modifiedTime = input_archive.itemProperty( item_index, BitProperty::MTime );

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

bool ProcessedItem::areAttributesDefined() const {
    return mAreAttributesDefined;
}

bool ProcessedItem::isModifiedTimeDefined() const {
    return mIsModifiedTimeDefined;
}

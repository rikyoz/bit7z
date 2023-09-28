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

#include "bitexception.hpp"
#include "internal/processeditem.hpp"

namespace bit7z {

ProcessedItem::ProcessedItem()
    : mModifiedTime{}, mIsModifiedTimeDefined{ false }, mAttributes{ 0 }, mAreAttributesDefined{ false } {}

void ProcessedItem::loadItemInfo( const BitInputArchive& inputArchive, std::uint32_t itemIndex ) {
    loadFilePath( inputArchive, itemIndex );
    loadAttributes( inputArchive, itemIndex );
    loadModifiedTime( inputArchive, itemIndex );
}

auto ProcessedItem::path() const -> fs::path {
    return mFilePath;
}

auto ProcessedItem::attributes() const -> uint32_t {
    return mAttributes;
}

auto ProcessedItem::modifiedTime() const -> FILETIME {
    return mModifiedTime;
}

void ProcessedItem::loadFilePath( const BitInputArchive& inputArchive, uint32_t itemIndex ) {
    const BitPropVariant prop = inputArchive.itemProperty( itemIndex, BitProperty::Path );

    switch ( prop.type() ) {
        case BitPropVariantType::Empty:
            mFilePath = fs::path{};
            break;

        case BitPropVariantType::String:
            mFilePath = fs::path{ prop.getNativeString() };
            break;

        default:
            throw BitException( "Could not load file path information of item", make_hresult_code( E_FAIL ) );
    }
}

void ProcessedItem::loadAttributes( const BitInputArchive& inputArchive, uint32_t itemIndex ) {
    mAttributes = 0;
    mAreAttributesDefined = false;

    // Get posix attributes
    const BitPropVariant posixAttributes = inputArchive.itemProperty( itemIndex, BitProperty::PosixAttrib );
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
    const BitPropVariant attributes = inputArchive.itemProperty( itemIndex, BitProperty::Attrib );
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

void ProcessedItem::loadModifiedTime( const BitInputArchive& inputArchive, uint32_t itemIndex ) {
    const BitPropVariant modifiedTime = inputArchive.itemProperty( itemIndex, BitProperty::MTime );

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

auto ProcessedItem::areAttributesDefined() const -> bool {
    return mAreAttributesDefined;
}

auto ProcessedItem::isModifiedTimeDefined() const -> bool {
    return mIsModifiedTimeDefined;
}

} // namespace bit7z
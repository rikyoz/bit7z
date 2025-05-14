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

#include "internal/processeditem.hpp"

#include "bitexception.hpp"
#include "bitinputarchive.hpp"
#include "bitpropvariant.hpp"
#include "internal/extractcallback.hpp"
#include "internal/fsutil.hpp"
#include "internal/windows.hpp"

#include <cstdint>

namespace bit7z {

ProcessedItem::ProcessedItem( const BitInputArchive& inputArchive, std::uint32_t itemIndex )
    : mAttributes{ 0 }, mAreAttributesDefined{ false } {
    loadFilePath( inputArchive, itemIndex );
    loadAttributes( inputArchive, itemIndex );
    loadTimeMetadata( inputArchive, itemIndex );
}

auto ProcessedItem::path() const -> fs::path {
    return mFilePath;
}

auto ProcessedItem::attributes() const -> std::uint32_t {
    return mAttributes;
}

auto ProcessedItem::modifiedTime() const -> FILETIME {
    return mModifiedTime.isFileTime() ? mModifiedTime.getFileTime() : FILETIME{};
}

#ifdef _WIN32
auto ProcessedItem::creationTime() const -> FILETIME {
    return mCreationTime.isFileTime() ? mCreationTime.getFileTime() : FILETIME{};
}

auto ProcessedItem::accessTime() const -> FILETIME {
    return mAccessTime.isFileTime() ? mAccessTime.getFileTime() : FILETIME{};
}
#else
auto ProcessedItem::hasModifiedTime() const -> bool {
    return mModifiedTime.isFileTime();
}
#endif

void ProcessedItem::loadFilePath( const BitInputArchive& inputArchive, std::uint32_t itemIndex ) {
    const BitPropVariant prop = inputArchive.itemProperty( itemIndex, BitProperty::Path );

    if ( !prop.isString() && !prop.isEmpty() ) {
        throw BitException( "Could not load file path information of item", make_hresult_code( E_FAIL ) );
    }

    mFilePath = prop.getNativeString();
    if ( mFilePath.empty() ) {
        const auto archivePath = tstring_to_path( inputArchive.archivePath() );
        mFilePath = !archivePath.empty() ? archivePath.stem() : fs::path{ kEmptyFileAlias };
    } else if ( !inputArchive.handler().retainDirectories() ) {
        mFilePath = mFilePath.filename();
    } else {
        // No action needed
    }
}

void ProcessedItem::loadAttributes( const BitInputArchive& inputArchive, std::uint32_t itemIndex ) {
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

void ProcessedItem::loadTimeMetadata( const BitInputArchive& inputArchive, std::uint32_t itemIndex ) {
    mModifiedTime = inputArchive.itemProperty( itemIndex, BitProperty::MTime );
#ifdef _WIN32
    mCreationTime = inputArchive.itemProperty( itemIndex, BitProperty::CTime );
    mAccessTime = inputArchive.itemProperty( itemIndex, BitProperty::ATime );
#endif
}

auto ProcessedItem::areAttributesDefined() const -> bool {
    return mAreAttributesDefined;
}

} // namespace bit7z
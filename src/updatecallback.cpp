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

#include "../include/updatecallback.hpp"

#include "../include/genericitem.hpp"
#include "../include/cfileoutstream.hpp"
#include "../include/util.hpp"

using namespace bit7z;

UpdateCallback::UpdateCallback( const BitArchiveCreator& creator, const ItemsIndex& new_items )
    : Callback{ creator },
      mNewItems{ new_items },
      mVolSize{ 0 },
      mOldArc{ nullptr },
      mOldArcItemsCount{ 0 },
      mRenamedItems{ nullptr },
      mUpdatedItems{ nullptr },
      mDeletedItems{ nullptr },
      mDeletedItemsCount{ 0 },
      mAskPassword{ false },
      mNeedBeClosed{ false } {}

UpdateCallback::~UpdateCallback() {
    Finalize();
}

void UpdateCallback::setOldArc( const BitInputArchive* old_arc ) {
    if ( old_arc != nullptr ) {
        mOldArc = old_arc;
        mOldArcItemsCount = old_arc->itemsCount();
    }
}

void UpdateCallback::setRenamedItems( const RenamedItems& renamed_items ) {
    mRenamedItems = &renamed_items;
}

void UpdateCallback::throwException( HRESULT error ) {
    if ( !mFailedFiles.empty() ) {
        throw BitException( "Error compressing files", make_hresult_code( error ), std::move( mFailedFiles ) );
    }
    Callback::throwException( error );
}

HRESULT UpdateCallback::Finalize() {
    if ( mNeedBeClosed ) {
        mNeedBeClosed = false;
    }

    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP UpdateCallback::SetTotal( UInt64 size ) {
    if ( mHandler.totalCallback() ) {
        mHandler.totalCallback()( size );
    }
    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP UpdateCallback::SetCompleted( const UInt64* completeValue ) {
    if ( completeValue != nullptr && mHandler.progressCallback() ) {
        return mHandler.progressCallback()( *completeValue ) ? S_OK : E_ABORT;
    }
    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP UpdateCallback::SetRatioInfo( const UInt64* inSize, const UInt64* outSize ) {
    if ( inSize != nullptr && outSize != nullptr && mHandler.ratioCallback() ) {
        mHandler.ratioCallback()( *inSize, *outSize );
    }
    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP UpdateCallback::GetProperty( UInt32 index, PROPID propID, PROPVARIANT* value ) {
    BitPropVariant prop;
    if ( propID == kpidIsAnti ) {
        prop = false;
    } else if ( index < mOldArcItemsCount - mDeletedItemsCount ) {
        if ( mDeletedItems != nullptr && mDeletedItems->find( index + mDeletedItemsCount ) == mDeletedItems->end() ) { // Not deleted
            if ( mRenamedItems != nullptr && propID == kpidPath ) { // Renamed by the user
                auto res = mRenamedItems->find( index + mDeletedItemsCount );
                if ( res != mRenamedItems->end() ) {
                    prop = WIDEN( res->second );
                }
            }
            if ( prop.isEmpty() && mUpdatedItems != nullptr ) { // Updated by the user
                auto res = mUpdatedItems->find( index + mDeletedItemsCount );
                if ( res != mUpdatedItems->end() ) {
                    prop = res->second->getProperty( propID );
                }
            }
            if ( prop.isEmpty() ) { // Not renamed or updated by the user
                prop = mOldArc->getItemProperty( index + mDeletedItemsCount, static_cast< BitProperty >( propID ) );
            }
        }
    } else {
        prop = getNewItemProperty( index + mDeletedItemsCount, propID );
    }
    *value = prop;
    prop.bstrVal = nullptr;
    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP UpdateCallback::GetStream( UInt32 index, ISequentialInStream** inStream ) {
    RINOK( Finalize() )

    auto old_index = index + mDeletedItemsCount;

    if ( old_index < mOldArcItemsCount ) { //old item in the archive
        if ( mUpdatedItems != nullptr ) {
            auto res = mUpdatedItems->find( old_index );
            if ( res != mUpdatedItems->end() ) { //user wants to update the old item in the archive
                return res->second->getStream( inStream );
            }
        }
        return S_OK;
    }

    return getNewItemStream( old_index, inStream );
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP UpdateCallback::GetVolumeSize( UInt32 /*index*/, UInt64* size ) {
    if ( mVolSize == 0 ) { return S_FALSE; }

    *size = mVolSize;
    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP UpdateCallback::GetVolumeStream( UInt32 index, ISequentialOutStream** volumeStream ) {
    tstring res = to_tstring( index + 1 );
    if ( res.length() < 3 ) {
        //adding leading zeros for a total res length of 3 (e.g. volume 42 will have extension .042)
        res.insert( res.begin(), 3 - res.length(), TSTRING( '0' ) );
    }

    tstring fileName = TSTRING( '.' ) + res;// + mVolExt;
    CMyComPtr< CFileOutStream > stream = new CFileOutStream( fileName );

    if ( stream->fail() ) {
        return HRESULT_FROM_WIN32( ERROR_OPEN_FAILED );
    }

    *volumeStream = stream.Detach();
    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP UpdateCallback::GetUpdateItemInfo( UInt32 index,
                                                Int32* newData,
                                                Int32* newProperties,
                                                UInt32* indexInArchive ) {

    if ( mDeletedItems != nullptr ) {
        for ( auto it = mDeletedItems->find( index + mDeletedItemsCount );
              it != mDeletedItems->end();
              ++it ) {
            ++mDeletedItemsCount;
        }
    }

    uint32_t old_index = index + mDeletedItemsCount;
    bool isOldItem = old_index < mOldArcItemsCount;
    bool isRenamedItem = mRenamedItems != nullptr && mRenamedItems->find( old_index ) != mRenamedItems->end();
    bool isUpdatedItem = mUpdatedItems != nullptr && mUpdatedItems->find( old_index ) != mUpdatedItems->end();

    if ( newData != nullptr ) {
        *newData = isOldItem && !isUpdatedItem ? 0 : 1; //= true;
    }
    if ( newProperties != nullptr ) {
        *newProperties = isOldItem && !isRenamedItem && !isUpdatedItem ? 0 : 1; //= true;
    }
    if ( indexInArchive != nullptr ) {
        *indexInArchive = isOldItem ? old_index : static_cast< uint32_t >( -1 );
    }

    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP UpdateCallback::SetOperationResult( Int32 /* operationResult */ ) {
    mNeedBeClosed = true;
    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP UpdateCallback::CryptoGetTextPassword2( Int32* passwordIsDefined, BSTR* password ) {
    if ( !mHandler.isPasswordDefined() ) {
        if ( mAskPassword ) {
            // You can ask real password here from user
            // Password = GetPassword(OutStream);
            // PasswordIsDefined = true;
            mErrorMessage = kPasswordNotDefined;
            return E_ABORT;
        }
    }

    *passwordIsDefined = ( mHandler.isPasswordDefined() ? 1 : 0 );
    return StringToBstr( WIDEN( mHandler.password() ).c_str(), password );
}

uint32_t UpdateCallback::itemsCount() const {
    auto result = mOldArcItemsCount + static_cast< uint32_t >( mNewItems.size() );
    if ( mDeletedItems != nullptr ) {
        result -= static_cast< uint32_t >( mDeletedItems->size() );
    }
    return result;
}

BitPropVariant UpdateCallback::getNewItemProperty( UInt32 realIndex, PROPID propID ) {
    const GenericItem& new_item = mNewItems[ static_cast< size_t >( realIndex - mOldArcItemsCount ) ];
    return new_item.getProperty( propID );
}

HRESULT UpdateCallback::getNewItemStream( uint32_t realIndex, ISequentialInStream** inStream ) {
    const GenericItem& new_item = mNewItems[ static_cast< size_t >( realIndex - mOldArcItemsCount ) ];

    if ( mHandler.fileCallback() ) {
        mHandler.fileCallback()( new_item.name() );
    }

    HRESULT res = new_item.getStream( inStream );
    if ( FAILED( res ) ) {
        auto path = new_item.path();
        std::error_code ec;
        if ( fs::exists( path, ec ) ) {
            ec = std::make_error_code( std::errc::file_exists );
        }
        mFailedFiles.emplace_back( path.native(), ec );
    }
    return res;
}

void UpdateCallback::setUpdatedItems( const UpdatedItems& updated_items ) {
    mUpdatedItems = &updated_items;
}

void UpdateCallback::setDeletedItems( const DeletedItems& deleted_items ) {
    mDeletedItems = &deleted_items;

}

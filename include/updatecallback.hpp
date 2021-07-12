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

#ifndef UPDATECALLBACK_HPP
#define UPDATECALLBACK_HPP

#ifndef _WIN32
#include <include_windows/windows.h>  //Needed for WINAPI macro definition used in IArchive of p7zip
#endif

#include <7zip/Archive/IArchive.h>
#include <7zip/ICoder.h>
#include <7zip/IPassword.h>

#include "../include/callback.hpp"
#include "../include/itemsindex.hpp"
#include "../include/bitarchivecreator.hpp"
#include "../include/bitinputarchive.hpp"
#include "../include/bitexception.hpp"

namespace bit7z {
    constexpr auto kUnsupportedOperation = "Unsupported operation";
    constexpr auto kUnsupportedInMemoryFormat = "Unsupported format for in-memory compression";
    constexpr auto kCannotOverwriteBuffer = "Cannot overwrite or update a non empty buffer";

    class UpdateCallback : public Callback,
                           public IArchiveUpdateCallback2,
                           public ICompressProgressInfo,
                           protected ICryptoGetTextPassword2 {
        public:
            explicit UpdateCallback( const BitArchiveCreator& creator, const ItemsIndex& new_items );

            ~UpdateCallback() override;

            MY_UNKNOWN_IMP3( IArchiveUpdateCallback2, ICompressProgressInfo, ICryptoGetTextPassword2 )

            uint32_t itemsCount() const;

            void setOldArc( const BitInputArchive* old_arc );

            void setRenamedItems( const RenamedItems& renamed_items );

            void setUpdatedItems( const UpdatedItems& updated_items );

            void setDeletedItems( const DeletedItems& deleted_items );

            BitPropVariant getNewItemProperty( UInt32 realIndex, PROPID propID );

            HRESULT getNewItemStream( uint32_t realIndex, ISequentialInStream** inStream );

            void throwException( HRESULT error ) override;

            void updateItemsOffsets();

            HRESULT Finalize();

            // IProgress from IArchiveUpdateCallback2
            STDMETHOD( SetTotal )( UInt64 size ) override;

            STDMETHOD( SetCompleted )( const UInt64* completeValue ) override;

            // ICompressProgressInfo
            STDMETHOD( SetRatioInfo )( const UInt64* inSize, const UInt64* outSize ) override;

            // IArchiveUpdateCallback2
            STDMETHOD( GetProperty )( UInt32 index, PROPID propID, PROPVARIANT* value ) override;

            STDMETHOD( GetStream )( UInt32 index, ISequentialInStream** inStream ) override;

            STDMETHOD( GetVolumeSize )( UInt32 index, UInt64* size ) override;

            STDMETHOD( GetVolumeStream )( UInt32 index, ISequentialOutStream** volumeStream ) override;

            STDMETHOD( GetUpdateItemInfo )( UInt32 index,
                                            Int32* newData,
                                            Int32* newProperties,
                                            UInt32* indexInArchive ) override;

            STDMETHOD( SetOperationResult )( Int32 operationResult ) override;

            //ICryptoGetTextPassword2
            STDMETHOD( CryptoGetTextPassword2 )( Int32* passwordIsDefined, BSTR* password ) override;

        private:
            const ItemsIndex& mNewItems;
            uint64_t mVolSize;

            const BitInputArchive* mOldArc;
            uint32_t mOldArcItemsCount;
            const RenamedItems* mRenamedItems; //note: using non-owning pointer on purpose
            const UpdatedItems* mUpdatedItems;
            const DeletedItems* mDeletedItems;
            
            std::vector< uint32_t > mItemsOffsets;

            bool mAskPassword;
            bool mNeedBeClosed;
            FailedFiles mFailedFiles;

            uint32_t getItemOffset( uint32_t index );
    };
}

#endif // UPDATECALLBACK_HPP

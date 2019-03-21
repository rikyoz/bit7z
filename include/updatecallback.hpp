/*
 * bit7z - A C++ static library to interface with the 7-zip DLLs.
 * Copyright (c) 2014-2018  Riccardo Ostani - All Rights Reserved.
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

#include "7zip/Archive/IArchive.h"
#include "7zip/ICoder.h"
#include "7zip/IPassword.h"
#include "Common/MyCom.h"

#include "../include/bitinputarchive.hpp"
#include "../include/bitarchiveitem.hpp"
#include "../include/compresscallback.hpp"
#include "../include/fsitem.hpp"
#include "../include/bitarchivecreator.hpp"

#include <vector>

namespace bit7z {
    using namespace filesystem;
    using std::pair;
    using std::vector;
    using std::wstring;

    class UpdateCallback : public CompressCallback,
                           public ICompressProgressInfo,
                           ICryptoGetTextPassword2,
                           CMyUnknownImp {
        public:
            vector< pair< wstring, HRESULT > > mFailedFiles;

            explicit UpdateCallback( const BitArchiveCreator& creator,
                                     const vector< FSItem >& new_items,
                                     const BitInputArchive* old_arc = nullptr );

            virtual ~UpdateCallback();

            // CompressCallback
            uint32_t itemsCount() const override;
            wstring getErrorMessage() const override;

            HRESULT Finilize();

            MY_UNKNOWN_IMP3( IArchiveUpdateCallback2, ICompressProgressInfo, ICryptoGetTextPassword2 )

            // IProgress
            STDMETHOD( SetTotal )( UInt64 size );
            STDMETHOD( SetCompleted )( const UInt64* completeValue );

            // ICompressProgressInfo
            STDMETHOD( SetRatioInfo )( const UInt64* inSize, const UInt64* outSize );

            // IArchiveUpdateCallback2
            STDMETHOD( EnumProperties )( IEnumSTATPROPSTG** enumerator );
            STDMETHOD( GetUpdateItemInfo )( UInt32 index,
                                            Int32* newData,
                                            Int32* newProperties,
                                            UInt32* indexInArchive );
            STDMETHOD( GetProperty )( UInt32 index, PROPID propID, PROPVARIANT* value );
            STDMETHOD( GetStream )( UInt32 index, ISequentialInStream** inStream );
            STDMETHOD( SetOperationResult )( Int32 operationResult );
            STDMETHOD( GetVolumeSize )( UInt32 index, UInt64* size );
            STDMETHOD( GetVolumeStream )( UInt32 index, ISequentialOutStream** volumeStream );

            //ICryptoGetTextPassword2
            STDMETHOD( CryptoGetTextPassword2 )( Int32* passwordIsDefined, BSTR* password );

        private:
            uint64_t mVolSize;
            wstring mVolName;

            const vector< FSItem >& mNewItems;
            const BitInputArchive* mOldArc;
            const uint32_t mOldArcItemsCount;
            const BitArchiveCreator& mCreator;

            bool mAskPassword;

            bool mNeedBeClosed;
    };
}
#endif // UPDATECALLBACK_HPP

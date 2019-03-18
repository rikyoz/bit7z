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

#ifndef MEMUPDATECALLBACK_HPP
#define MEMUPDATECALLBACK_HPP

#include "7zip/Archive/IArchive.h"
#include "7zip/IPassword.h"
#include "Common/MyCom.h"

#include "../include/bitarchivecreator.hpp"
#include "../include/bitinputarchive.hpp"
#include "../include/bittypes.hpp"
#include "../include/callback.hpp"

#include <vector>

namespace bit7z {
    using std::vector;
    using std::wstring;

    class MemUpdateCallback : public IArchiveUpdateCallback, ICryptoGetTextPassword2, CMyUnknownImp, public Callback {
        public:
            MY_UNKNOWN_IMP2( IArchiveUpdateCallback, ICryptoGetTextPassword2 )

            // IProgress
            STDMETHOD( SetTotal )( UInt64 size );
            STDMETHOD( SetCompleted )( const UInt64* completeValue );

            // IArchiveUpdateCallback
            STDMETHOD( EnumProperties )( IEnumSTATPROPSTG** enumerator );
            STDMETHOD( GetUpdateItemInfo )( UInt32 index,
                                            Int32* newData,
                                            Int32* newProperties,
                                            UInt32* indexInArchive );
            STDMETHOD( GetProperty )( UInt32 /*index*/, PROPID propID, PROPVARIANT* value );
            STDMETHOD( GetStream )( UInt32 /*index*/, ISequentialInStream** inStream );
            STDMETHOD( SetOperationResult )( Int32 operationResult );

            //ICryptoGetTextPassword2
            STDMETHOD( CryptoGetTextPassword2 )( Int32* passwordIsDefined, BSTR* password );

            uint32_t getItemsCount() const;

        public:
            const BitArchiveCreator& mCreator;
            /*vector< UInt64 > mVolumesSizes;
            wstring mVolName;
            wstring mVolExt;

            wstring mDirPrefix;*/

            bool mAskPassword;

            bool mNeedBeClosed;

            const vector< byte_t >& mBuffer;
            const wstring& mBufferName;
            const BitInputArchive* mOldArc;
            const uint32_t mOldArcItemsCount;

            MemUpdateCallback( const BitArchiveCreator& creator,
                               const vector< byte_t >& out_buffer,
                               const wstring& buffer_name ,
                               const BitInputArchive *old_arc );

            virtual ~MemUpdateCallback();

            HRESULT Finilize();
    };
}
#endif // MEMUPDATECALLBACK_HPP

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

#ifndef MEMEXTRACTCALLBACK_HPP
#define MEMEXTRACTCALLBACK_HPP

#include <string>
#include <vector>

#include "7zip/Archive/IArchive.h"
#include "7zip/Common/FileStreams.h"
#include "7zip/Common/StreamObjects.h"
#include "7zip/IPassword.h"
#include "Common/MyCom.h"

#include "../include/coutmemstream.hpp"
#include "../include/bitguids.hpp"
#include "../include/bitformat.hpp"
#include "../include/bittypes.hpp"
#include "../include/callback.hpp"
#include "../include/bitarchiveopener.hpp"

namespace bit7z {
    using std::vector;

    class MemExtractCallback : public IArchiveExtractCallback, ICryptoGetTextPassword, CMyUnknownImp, public Callback {
        public:
            MemExtractCallback( const BitArchiveOpener& opener, IInArchive* archiveHandler, vector< byte_t >& buffer );
            virtual ~MemExtractCallback();

            MY_UNKNOWN_IMP1( ICryptoGetTextPassword )

            // IProgress
            STDMETHOD( SetTotal )( UInt64 size );
            STDMETHOD( SetCompleted )( const UInt64 * completeValue );

            // IArchiveExtractCallback
            STDMETHOD( GetStream )( UInt32 index, ISequentialOutStream * *outStream, Int32 askExtractMode );
            STDMETHOD( PrepareOperation )( Int32 askExtractMode );
            STDMETHOD( SetOperationResult )( Int32 resultEOperationResult );

            // ICryptoGetTextPassword
            STDMETHOD( CryptoGetTextPassword )( BSTR * aPassword );

        private:
            const BitArchiveOpener& mOpener;
            CMyComPtr< IInArchive > mArchiveHandler;
            vector< byte_t >& mBuffer;
            bool mExtractMode;
            struct CProcessedFileInfo {
                FILETIME MTime;
                UInt32 Attrib;
                bool isDir;
                bool AttribDefined;
                bool MTimeDefined;
            } mProcessedFileInfo;

            COutMemStream* mOutMemStreamSpec;
            CMyComPtr< ISequentialOutStream > mOutMemStream;

            UInt64 mNumErrors;
    };
}
#endif // MEMEXTRACTCALLBACK_HPP

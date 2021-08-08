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

#ifndef EXTRACTCALLBACK_HPP
#define EXTRACTCALLBACK_HPP

#ifndef _WIN32
#include <include_windows/windows.h>  //Needed for WINAPI macro definition used in IArchive of p7zip
#endif

#include <7zip/Archive/IArchive.h>
#include <7zip/ICoder.h>
#include <7zip/IPassword.h>

#include "bitarchivehandler.hpp"
#include "bitinputarchive.hpp"
#include "internal/callback.hpp"

namespace bit7z {
    class ExtractCallback : public Callback,
                            public IArchiveExtractCallback,
                            public ICompressProgressInfo,
                            protected ICryptoGetTextPassword {
        public:
            ExtractCallback( const ExtractCallback& ) = delete;

            ExtractCallback( ExtractCallback&& ) = delete;

            ExtractCallback& operator=( const ExtractCallback& ) = delete;

            ExtractCallback& operator=( ExtractCallback&& ) = delete;

            ~ExtractCallback() override = default;

            MY_UNKNOWN_IMP3( IArchiveExtractCallback, ICompressProgressInfo, ICryptoGetTextPassword )

            // IProgress from IArchiveExtractCallback
            STDMETHOD_OVERRIDE( SetTotal, UInt64 size );

            STDMETHOD_OVERRIDE( SetCompleted, const UInt64* completeValue );

            // ICompressProgressInfo
            STDMETHOD_OVERRIDE( SetRatioInfo, const UInt64* inSize, const UInt64* outSize );

            // IArchiveExtractCallback
            STDMETHOD_NOEXCEPT_OVERRIDE( PrepareOperation, Int32 askExtractMode );

            // ICryptoGetTextPassword
            STDMETHOD_OVERRIDE( CryptoGetTextPassword, BSTR* aPassword );

            // IArchiveExtractCallback
            STDMETHOD_OVERRIDE( GetStream, UInt32 index, ISequentialOutStream** outStream, Int32 askExtractMode );

            STDMETHOD_OVERRIDE( SetOperationResult, Int32 operationResult );

        protected:
            explicit ExtractCallback( const BitInputArchive& inputArchive );

            virtual void finishOperation();

            virtual void releaseStream() = 0;

            virtual HRESULT getOutStream( UInt32 index, ISequentialOutStream** outStream, int32_t askExtractMode ) = 0;

            const BitInputArchive& mInputArchive;

            bool mExtractMode;

            UInt64 mNumErrors;
    };
}

#endif // EXTRACTCALLBACK_HPP

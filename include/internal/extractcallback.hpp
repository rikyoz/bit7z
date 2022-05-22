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

#include "bitinputarchive.hpp"
#include "internal/callback.hpp"
#include "internal/util.hpp"

#include <7zip/Archive/IArchive.h>
#include <7zip/ICoder.h>
#include <7zip/IPassword.h>

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

            // NOLINTNEXTLINE(modernize-use-noexcept)
            MY_UNKNOWN_IMP3( IArchiveExtractCallback, ICompressProgressInfo, ICryptoGetTextPassword )

            // IProgress from IArchiveExtractCallback
            BIT7Z_STDMETHOD( SetTotal, UInt64 size );

            BIT7Z_STDMETHOD( SetCompleted, const UInt64* completeValue );

            // ICompressProgressInfo
            BIT7Z_STDMETHOD( SetRatioInfo, const UInt64* inSize, const UInt64* outSize );

            // IArchiveExtractCallback
            BIT7Z_STDMETHOD_NOEXCEPT( PrepareOperation, Int32 askExtractMode );

            // ICryptoGetTextPassword
            BIT7Z_STDMETHOD( CryptoGetTextPassword, BSTR* aPassword );

            // IArchiveExtractCallback
            BIT7Z_STDMETHOD( GetStream, UInt32 index, ISequentialOutStream** outStream, Int32 askExtractMode );

            BIT7Z_STDMETHOD( SetOperationResult, Int32 operationResult );

        protected:
            explicit ExtractCallback( const BitInputArchive& inputArchive );

            virtual void finishOperation();

            virtual void releaseStream() = 0;

            virtual HRESULT getOutStream( UInt32 index, ISequentialOutStream** outStream, int32_t askExtractMode ) = 0;

            const BitInputArchive& mInputArchive;

            bool mExtractMode;

            UInt64 mNumErrors;
    };
}  // namespace bit7z

#endif // EXTRACTCALLBACK_HPP

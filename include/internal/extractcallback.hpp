/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
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

        virtual HRESULT getOutStream( UInt32 index, ISequentialOutStream** outStream ) = 0;

        const BitInputArchive& mInputArchive;

        bool mExtractMode;

        UInt64 mNumErrors;
};
}  // namespace bit7z

#endif // EXTRACTCALLBACK_HPP

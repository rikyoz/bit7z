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
#include "internal/macros.hpp"

#include <7zip/Archive/IArchive.h>
#include <7zip/ICoder.h>
#include <7zip/IPassword.h>

using namespace NArchive::NExtract;

namespace bit7z {

enum struct ExtractMode {
    Extract = NAskMode::kExtract,
    Test = NAskMode::kTest,
    Skip = NAskMode::kSkip
};

enum struct OperationResult {
    Success = NOperationResult::kOK,
    UnsupportedMethod = NOperationResult::kUnsupportedMethod,
    DataError = NOperationResult::kDataError,
    CRCError = NOperationResult::kCRCError,
    Unavailable = NOperationResult::kUnavailable,
    UnexpectedEnd = NOperationResult::kUnexpectedEnd,
    DataAfterEnd = NOperationResult::kDataAfterEnd,
    IsNotArc = NOperationResult::kIsNotArc,
    HeadersError = NOperationResult::kHeadersError,
    WrongPassword = NOperationResult::kWrongPassword
};

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

        inline ExtractMode extractMode() const {
            return mExtractMode;
        }

        inline bool isItemFolder( uint32_t index ) const {
            return mInputArchive.isItemFolder( index );
        }

        inline BitPropVariant itemProperty( uint32_t index, BitProperty property ) const {
            return mInputArchive.itemProperty( index, property );
        }

        inline const BitInputArchive& inputArchive() const {
            return mInputArchive;
        }

        virtual HRESULT finishOperation( OperationResult operation_result );

        virtual void releaseStream() = 0;

        virtual HRESULT getOutStream( UInt32 index, ISequentialOutStream** outStream ) = 0;

    private:
        const BitInputArchive& mInputArchive;
        ExtractMode mExtractMode;
};

}  // namespace bit7z

#endif // EXTRACTCALLBACK_HPP

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

#include <system_error>

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
    WrongPassword = NOperationResult::kWrongPassword,
    DataErrorEncrypted = 2 * NOperationResult::kWrongPassword,
    CRCErrorEncrypted = 2 * NOperationResult::kWrongPassword + 1,
    OpenErrorEncrypted = 2 * NOperationResult::kWrongPassword + 2
};

auto make_error_code( OperationResult error ) -> std::error_code;

class ExtractCallback : public Callback,
                        public IArchiveExtractCallback,
                        public ICompressProgressInfo,
                        protected ICryptoGetTextPassword {
    public:
        ExtractCallback( const ExtractCallback& ) = delete;

        ExtractCallback( ExtractCallback&& ) = delete;

        auto operator=( const ExtractCallback& ) -> ExtractCallback& = delete;

        auto operator=( ExtractCallback&& ) -> ExtractCallback& = delete;

        ~ExtractCallback() override = default;

        // IProgress from IArchiveExtractCallback
        BIT7Z_STDMETHOD( SetTotal, UInt64 size );

        BIT7Z_STDMETHOD( SetCompleted, const UInt64* completeValue );

        // ICompressProgressInfo
        BIT7Z_STDMETHOD( SetRatioInfo, const UInt64* inSize, const UInt64* outSize );

        // IArchiveExtractCallback
        BIT7Z_STDMETHOD( PrepareOperation, Int32 askExtractMode );

        // ICryptoGetTextPassword
        BIT7Z_STDMETHOD( CryptoGetTextPassword, BSTR* aPassword );

        // IArchiveExtractCallback
        BIT7Z_STDMETHOD( GetStream, UInt32 index, ISequentialOutStream** outStream, Int32 askExtractMode );

        BIT7Z_STDMETHOD( SetOperationResult, Int32 operationResult );

        BIT7Z_NODISCARD
        inline auto errorException() const -> const std::exception_ptr& {
            return mErrorException;
        }

        // NOLINTNEXTLINE(modernize-use-noexcept, modernize-use-trailing-return-type, readability-identifier-length)
        MY_UNKNOWN_IMP3( IArchiveExtractCallback, ICompressProgressInfo, ICryptoGetTextPassword ) //-V2507 //-V2511

    protected:
        explicit ExtractCallback( const BitInputArchive& inputArchive );

        BIT7Z_NODISCARD
        inline auto extractMode() const -> ExtractMode {
            return mExtractMode;
        }

        BIT7Z_NODISCARD
        inline auto isItemFolder( uint32_t index ) const -> bool {
            return mInputArchive.isItemFolder( index );
        }

        BIT7Z_NODISCARD
        inline auto itemProperty( uint32_t index, BitProperty property ) const -> BitPropVariant {
            return mInputArchive.itemProperty( index, property );
        }

        BIT7Z_NODISCARD
        inline auto inputArchive() const -> const BitInputArchive& {
            return mInputArchive;
        }

        virtual auto finishOperation( OperationResult operation_result ) -> HRESULT;

        virtual void releaseStream() = 0;

        virtual auto getOutStream( UInt32 index, ISequentialOutStream** outStream ) -> HRESULT = 0;

    private:
        const BitInputArchive& mInputArchive;
        ExtractMode mExtractMode;
        bool mIsLastItemEncrypted;
        std::exception_ptr mErrorException;
};

}  // namespace bit7z

#endif // EXTRACTCALLBACK_HPP

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2024 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef RAWDATAEXTRACTCALLBACKWID_HPP
#define RAWDATAEXTRACTCALLBACKWID_HPP

#include "bitabstractarchivehandler.hpp"
#include "internal/extractcallback.hpp"

#include <cstdint>

namespace bit7z {

class BitInputArchive;


class RawDataExtractCallbackWid final : public ExtractCallback {
    public:
        using CallbackType = FileAwareExtraction;
        RawDataExtractCallbackWid( const BitInputArchive& inputArchive, CallbackType& callback );

        RawDataExtractCallbackWid( const RawDataExtractCallbackWid& ) = delete;

        RawDataExtractCallbackWid( RawDataExtractCallbackWid&& ) = delete;

        auto operator=( const RawDataExtractCallbackWid& ) -> RawDataExtractCallbackWid& = delete;

        auto operator=( RawDataExtractCallbackWid&& ) -> RawDataExtractCallbackWid& = delete;

        ~RawDataExtractCallbackWid() override = default;

    private:
        CallbackType& mCallback;
        CMyComPtr< ISequentialOutStream > mCallbackStream;

        void releaseStream() override;

        auto getOutStream( std::uint32_t index, ISequentialOutStream** outStream ) -> HRESULT override;
};

}  // namespace bit7z

#endif // RAWDATAEXTRACTCALLBACKWID_HPP

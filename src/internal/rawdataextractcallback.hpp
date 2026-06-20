/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef RAWDATAEXTRACTCALLBACK_HPP
#define RAWDATAEXTRACTCALLBACK_HPP

#include "bitabstractarchivehandler.hpp"
#include "internal/extractcallback.hpp"

namespace bit7z {

class BitInputArchive;

class RawDataExtractCallback final : public ExtractCallback {
    public:
        RawDataExtractCallback( const BitInputArchive& inputArchive, RawDataCallback callback );

        RawDataExtractCallback( const RawDataExtractCallback& ) = delete;

        RawDataExtractCallback( RawDataExtractCallback&& ) = delete;

        auto operator=( const RawDataExtractCallback& ) -> RawDataExtractCallback& = delete;

        auto operator=( RawDataExtractCallback&& ) -> RawDataExtractCallback& = delete;

        ~RawDataExtractCallback() override = default;

    private:
        RawDataCallback mCallback;
        CMyComPtr< ISequentialOutStream > mCallbackStream;

        void releaseStream() override;

        auto getOutStream( const BitArchiveItem& item, ISequentialOutStream** outStream ) -> HRESULT override;
};

} // namespace bit7z

#endif // RAWDATAEXTRACTCALLBACK_HPP

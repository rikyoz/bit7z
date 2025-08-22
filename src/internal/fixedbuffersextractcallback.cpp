/*
* bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "fixedbuffersextractcallback.hpp"


namespace bit7z {

FixedBuffersExtractCallback::FixedBuffersExtractCallback(const BitInputArchive &inputArchive, std::map<uint32_t, ByteSpan> buffers) :
    ExtractCallback(inputArchive),
    mBuffers(std::move(buffers)) {
     for (auto& [idx, buffer] : mBuffers) {
         mBuffersCallbacks.emplace(std::piecewise_construct, std::make_tuple(idx), std::make_tuple(std::ref(inputArchive), buffer.data(), buffer.size()));
     }
}

void FixedBuffersExtractCallback::releaseStream() {
    for (auto& [idx, callback] : mBuffersCallbacks) {
        callback.releaseStream();
    }
}

auto FixedBuffersExtractCallback::getOutStream(uint32_t index, ISequentialOutStream **outStream) -> HRESULT {
    if ( isItemFolder( index ) ) {
        return S_OK;
    }

    auto const found_it = mBuffersCallbacks.find(index);
    if (found_it == mBuffersCallbacks.end()) {
        return E_FAIL;
    }

    auto& callback = found_it->second;
    return callback.getOutStream(index, outStream);
}

}

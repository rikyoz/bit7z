/*
* bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2023 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef BYTESPAN_HPP
#define BYTESPAN_HPP

#include <bittypes.hpp>
#ifdef __cpp_lib_span
#include <span>
#ifdef __cpp_lib_byte
#include <cstddef>
#endif
#endif

namespace bit7z {

class ByteSpan {
public:
    ByteSpan() = default;
    ByteSpan(byte_t* ptr, std::size_t const size) : mMemoryPointer(ptr), mSize(size){}

#ifdef __cpp_lib_span
#ifdef __cpp_lib_byte
    /*implicit*/ ByteSpan(std::span<std::byte> view) : BitSpan(reinterpret_cast<byte_t*>(view.data(), view.size())) {}
#endif
    /*implicit*/ ByteSpan(std::span<char> view) : BitSpan(reinterpret_cast<char*>(view.data(), view.size())) {}
    /*implicit*/ ByteSpan(std::span<unsigned char> view) : : BitSpan(reinterpret_cast<unsigned char*>(view.data(), view.size())) {}
#endif

    byte_t* data() { return mMemoryPointer;}
    byte_t const* data() const { return mMemoryPointer;}
    std::size_t size() const { return mSize;}

    bool empty() const { return data() == nullptr;}
private:
    byte_t* mMemoryPointer = nullptr;
    std::size_t mSize = 0;
};

}

#endif //BYTESPAN_HPP

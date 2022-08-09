/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef CALLBACK_HPP
#define CALLBACK_HPP

#include <string>

#include "bitabstractarchivehandler.hpp"
#include "internal/guids.hpp"

#include <Common/MyCom.h>

constexpr auto kPasswordNotDefined = "Password is not defined";
constexpr auto kEmptyFileAlias = BIT7Z_STRING( "[Content]" );

namespace bit7z {
class Callback : protected CMyUnknownImp {
    public:
        Callback( const Callback& ) = delete;

        Callback( Callback&& ) = delete;

        Callback& operator=( const Callback& ) = delete;

        Callback& operator=( Callback&& ) = delete;

        virtual ~Callback() = default;

        virtual void throwException( HRESULT error );

    protected:
        explicit Callback( const BitAbstractArchiveHandler& handler ); // Abstract class

        const BitAbstractArchiveHandler& mHandler;
        const char* mErrorMessage;
};
}  // namespace bit7z
#endif // CALLBACK_HPP

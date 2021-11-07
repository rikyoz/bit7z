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

#ifndef OPENCALLBACK_HPP
#define OPENCALLBACK_HPP

#include "bitabstractarchivehandler.hpp"
#include "internal/callback.hpp"
#include "internal/fsitem.hpp"
#include "internal/util.hpp"

#include <7zip/Archive/IArchive.h>
#include <7zip/IPassword.h>
#include <Common/MyCom.h>

namespace bit7z {
    using filesystem::FSItem;

    class OpenCallback final : public IArchiveOpenCallback,
                               public IArchiveOpenVolumeCallback,
                               public IArchiveOpenSetSubArchiveName,
                               public ICryptoGetTextPassword,
                               public Callback {
        public:
            explicit OpenCallback( const BitAbstractArchiveHandler& handler, const tstring& filename = BIT7Z_STRING( "." ) );

            OpenCallback( const OpenCallback& ) = delete;

            OpenCallback( OpenCallback&& ) = delete;

            OpenCallback& operator=( const OpenCallback& ) = delete;

            OpenCallback& operator=( OpenCallback&& ) = delete;

            ~OpenCallback() override = default;

            MY_UNKNOWN_IMP3( IArchiveOpenVolumeCallback, IArchiveOpenSetSubArchiveName, ICryptoGetTextPassword ) // NOLINT(modernize-use-noexcept)

            //IArchiveOpenCallback
            BIT7Z_STDMETHOD_NOEXCEPT( SetTotal, const UInt64* files, const UInt64* bytes );

            BIT7Z_STDMETHOD_NOEXCEPT( SetCompleted, const UInt64* files, const UInt64* bytes );

            //IArchiveOpenVolumeCallback
            BIT7Z_STDMETHOD( GetProperty, PROPID propID, PROPVARIANT* value );

            BIT7Z_STDMETHOD( GetStream, const wchar_t* name, IInStream** inStream );

            //IArchiveOpenSetSubArchiveName
            BIT7Z_STDMETHOD( SetSubArchiveName, const wchar_t* name );

            //ICryptoGetTextPassword
            BIT7Z_STDMETHOD( CryptoGetTextPassword, BSTR* password );

        private:
            bool mSubArchiveMode;
            std::wstring mSubArchiveName;
            FSItem mFileItem;
    };
}
#endif // OPENCALLBACK_HPP

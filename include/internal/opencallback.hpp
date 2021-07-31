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

#ifndef _WIN32
#include <include_windows/windows.h>  //Needed for WINAPI macro definition used in IArchive of p7zip
#endif

#include <7zip/Archive/IArchive.h>
#include <7zip/IPassword.h>
#include <Common/MyCom.h>

#include "bitarchivehandler.hpp"
#include "internal/callback.hpp"
#include "internal/fsitem.hpp"

namespace bit7z {
    using filesystem::FSItem;

    class OpenCallback final : public IArchiveOpenCallback,
                               public IArchiveOpenVolumeCallback,
                               public IArchiveOpenSetSubArchiveName,
                               public ICryptoGetTextPassword,
                               public Callback {
        public:
            explicit OpenCallback( const BitArchiveHandler& handler, const tstring& filename = TSTRING( "." ) );

            ~OpenCallback() override = default;

            MY_UNKNOWN_IMP3( IArchiveOpenVolumeCallback, IArchiveOpenSetSubArchiveName, ICryptoGetTextPassword )

            //IArchiveOpenCallback
            STDMETHOD_NOEXCEPT_OVERRIDE( SetTotal, const UInt64* files, const UInt64* bytes );

            STDMETHOD_NOEXCEPT_OVERRIDE( SetCompleted, const UInt64* files, const UInt64* bytes );

            //IArchiveOpenVolumeCallback
            STDMETHOD_OVERRIDE( GetProperty, PROPID propID, PROPVARIANT* value );

            STDMETHOD_OVERRIDE( GetStream, const wchar_t* name, IInStream** inStream );

            //IArchiveOpenSetSubArchiveName
            STDMETHOD_OVERRIDE( SetSubArchiveName, const wchar_t* name );

            //ICryptoGetTextPassword
            STDMETHOD_OVERRIDE( CryptoGetTextPassword, BSTR* password );

        private:
            bool mSubArchiveMode;
            std::wstring mSubArchiveName;
            FSItem mFileItem;
    };
}
#endif // OPENCALLBACK_HPP

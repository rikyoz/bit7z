/*
 * bit7z - A C++ static library to interface with the 7-zip DLLs.
 * Copyright (c) 2014-2019  Riccardo Ostani - All Rights Reserved.
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

#ifndef EXTRACTCALLBACK_HPP
#define EXTRACTCALLBACK_HPP

#ifndef _WIN32
#include <include_windows/windows.h>  //Needed for WINAPI macro definition used in IArchive of p7zip
#endif

#include <7zip/Archive/IArchive.h>
#include <7zip/ICoder.h>
#include <7zip/IPassword.h>

#include "../include/bitarchivehandler.hpp"
#include "../include/bitinputarchive.hpp"
#include "../include/callback.hpp"

namespace bit7z {
    class ExtractCallback : public Callback,
                            public IArchiveExtractCallback,
                            public ICompressProgressInfo,
                            protected ICryptoGetTextPassword {
        public:
            ~ExtractCallback() override = default;

            MY_UNKNOWN_IMP3( IArchiveExtractCallback, ICompressProgressInfo, ICryptoGetTextPassword )

            // IProgress from IArchiveExtractCallback
            STDMETHOD( SetTotal )( UInt64 size ) override;

            STDMETHOD( SetCompleted )( const UInt64* completeValue ) override;

            // ICompressProgressInfo
            STDMETHOD( SetRatioInfo )( const UInt64* inSize, const UInt64* outSize ) override;

            // IArchiveExtractCallback
            STDMETHOD( PrepareOperation )( Int32 askExtractMode ) override;

            // ICryptoGetTextPassword
            STDMETHOD( CryptoGetTextPassword )( BSTR* aPassword ) override;

        protected:
            ExtractCallback( const BitArchiveHandler& handler,
                             const BitInputArchive& inputArchive );

            const BitInputArchive& mInputArchive;

            bool mExtractMode;

            UInt64 mNumErrors;
    };
}

#endif // EXTRACTCALLBACK_HPP

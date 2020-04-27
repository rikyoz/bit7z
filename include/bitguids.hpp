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

#ifndef BITGUIDS_HPP
#define BITGUIDS_HPP

#ifdef _WIN32
#include <guiddef.h>
#else
#include <Common/MyGuidDef.h>
#endif

namespace bit7z {
    extern "C" {
#if !defined(_WIN32)
        extern const GUID IID_IUnknown;
#endif

        // IStream.h
        extern const GUID IID_ISequentialInStream;
        extern const GUID IID_ISequentialOutStream;
        extern const GUID IID_IInStream;
        extern const GUID IID_IOutStream;
        extern const GUID IID_IStreamGetSize;
        extern const GUID IID_IStreamGetProps;
        extern const GUID IID_IStreamGetProps2;

        // ICoder.h
        extern const GUID IID_ICompressProgressInfo;

        // IPassword.h
        extern const GUID IID_ICryptoGetTextPassword;
        extern const GUID IID_ICryptoGetTextPassword2;

        // IArchive.h
        extern const GUID IID_ISetProperties;
        extern const GUID IID_IInArchive;
        extern const GUID IID_IOutArchive;
        extern const GUID IID_IArchiveExtractCallback;
        extern const GUID IID_IArchiveOpenVolumeCallback;
        extern const GUID IID_IArchiveOpenSetSubArchiveName;
        extern const GUID IID_IArchiveUpdateCallback;
        extern const GUID IID_IArchiveUpdateCallback2;
    }
}
#endif // BITGUIDS_HPP

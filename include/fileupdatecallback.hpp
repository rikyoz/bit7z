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

#ifndef FILEUPDATECALLBACK_HPP
#define FILEUPDATECALLBACK_HPP

#include "../include/bitinputarchive.hpp"
#include "../include/bitarchiveiteminfo.hpp"
#include "../include/updatecallback.hpp"
#include "../include/fsitem.hpp"
#include "../include/bitarchivecreator.hpp"
#include "../include/bitexception.hpp"

#include <vector>

namespace bit7z {
    using namespace filesystem;
    using std::pair;
    using std::vector;
    using std::wstring;

    class FileUpdateCallback : public UpdateCallback {
        public:
            explicit FileUpdateCallback( const BitArchiveCreator& creator, const vector< FSItem >& new_items );

            ~FileUpdateCallback() override = default;

            // CompressCallback
            uint32_t itemsCount() const override;

            void throwException( HRESULT error ) override;

            // IArchiveUpdateCallback2
            STDMETHOD( GetProperty )( UInt32 index, PROPID propID, PROPVARIANT* value ) override;

            STDMETHOD( GetStream )( UInt32 index, ISequentialInStream** inStream ) override;

            STDMETHOD( GetVolumeSize )( UInt32 index, UInt64* size ) override;

            STDMETHOD( GetVolumeStream )( UInt32 index, ISequentialOutStream** volumeStream ) override;

        private:
            const vector< FSItem >& mNewItems;

            uint64_t mVolSize;
            tstring mVolName; //TODO: Check whether this is necessary...

            FailedFiles mFailedFiles;
    };
}
#endif // FILEUPDATECALLBACK_HPP

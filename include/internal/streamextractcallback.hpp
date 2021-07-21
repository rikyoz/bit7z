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

#ifndef STREAMEXTRACTCALLBACK_HPP
#define STREAMEXTRACTCALLBACK_HPP

#include <vector>
#include <map>

#include "bittypes.hpp"
#include "internal/extractcallback.hpp"

namespace bit7z {
    using std::vector;
    using std::map;
    using std::ostream;

    class StreamExtractCallback : public ExtractCallback {
        public:
            StreamExtractCallback( const BitInputArchive& inputArchive, ostream& outputStream );

            ~StreamExtractCallback() override = default;

            void throwException( HRESULT error ) override;

        private:
            ostream& mOutputStream;
            CMyComPtr< IOutStream > mStdOutStream;

            void releaseStream() override;

            HRESULT getOutStream( uint32_t index, ISequentialOutStream** outStream, int32_t askExtractMode ) override;
    };
}
#endif // STREAMEXTRACTCALLBACK_HPP

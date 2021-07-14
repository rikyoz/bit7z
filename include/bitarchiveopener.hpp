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

#ifndef BITARCHIVEOPENER_HPP
#define BITARCHIVEOPENER_HPP

#include <vector>
#include <map>

#include "bitarchivehandler.hpp"
#include "bitformat.hpp"
#include "bittypes.hpp"

namespace bit7z {
    using std::vector;
    using std::map;
    using std::ostream;

    class BitInputArchive;

    /**
     * @brief Abstract class representing a generic archive opener.
     */
    class BitArchiveOpener : public BitArchiveHandler {
        public:

            /**
             * @return the archive format used by the archive opener.
             */
            const BitInFormat& format() const override;

            /**
             * @return the archive format used by the archive opener.
             */
            const BitInFormat& extractionFormat() const;

        protected:
            const BitInFormat& mFormat;

            BitArchiveOpener( const Bit7zLibrary& lib,
                              const BitInFormat& format,
                              const tstring& password = TSTRING( "" ) );

            ~BitArchiveOpener() override = default;

    };
}

#endif // BITARCHIVEOPENER_HPP

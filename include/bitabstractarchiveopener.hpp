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

#ifndef BITABSTRACTARCHIVEOPENER_HPP
#define BITABSTRACTARCHIVEOPENER_HPP

#include <vector>
#include <map>

#include "bitabstractarchivehandler.hpp"
#include "bitformat.hpp"

namespace bit7z {
    using std::vector;
    using std::map;
    using std::ostream;

    /**
     * @brief The BitAbstractArchiveOpener abstract class represents a generic archive opener.
     */
    class BitAbstractArchiveOpener : public BitAbstractArchiveHandler {
        public:
            BitAbstractArchiveOpener( const BitAbstractArchiveOpener& ) = delete;

            BitAbstractArchiveOpener( BitAbstractArchiveOpener&& ) = delete;

            BitAbstractArchiveOpener& operator=( const BitAbstractArchiveOpener& ) = delete;

            BitAbstractArchiveOpener& operator=( BitAbstractArchiveOpener&& ) = delete;

            /**
             * @return the archive format used by the archive opener.
             */
            BIT7Z_NODISCARD const BitInFormat& format() const noexcept override;

            /**
             * @return the archive format used by the archive opener.
             */
            BIT7Z_NODISCARD const BitInFormat& extractionFormat() const noexcept;

        protected:
            const BitInFormat& mFormat;

            BitAbstractArchiveOpener( const Bit7zLibrary& lib,
                                      const BitInFormat& format,
                                      const tstring& password = {} );

            ~BitAbstractArchiveOpener() override = default;
    };
}  // namespace bit7z

#endif // BITABSTRACTARCHIVEOPENER_HPP

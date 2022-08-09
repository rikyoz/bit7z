/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef BITABSTRACTARCHIVEOPENER_HPP
#define BITABSTRACTARCHIVEOPENER_HPP

#include <vector>
#include <map>

#include "bitabstractarchivehandler.hpp"
#include "bitformat.hpp"

namespace bit7z {
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

        ~BitAbstractArchiveOpener() override = default;

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
};
}  // namespace bit7z

#endif // BITABSTRACTARCHIVEOPENER_HPP
